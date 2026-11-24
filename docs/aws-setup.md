# AWS setup (one-time, in the console)

This sets up hosting for the WebAssembly build: a **private S3 bucket** holding the
files, **CloudFront** serving them over HTTPS, and an **IAM role** that GitHub
Actions assumes through OIDC to deploy. No long-lived AWS keys exist anywhere.

```
GitHub (push to main) → Actions: build with Emscripten
                      → assume IAM role via OIDC (no stored keys)
                      → upload to private S3 bucket
                      → invalidate CloudFront cache
Player → https://<id>.cloudfront.net → CloudFront → S3 (via Origin Access Control)
```

Throughout, replace:

| Placeholder | Example | Where you get it |
|---|---|---|
| `<ACCOUNT_ID>` | `123456789012` | Console, top-right account menu |
| `<BUCKET>` | `breakout-game-a1b2` | You choose it in step 2 |
| `<DISTRIBUTION_ID>` | `E1ABCDEF2GHIJK` | Shown after step 3 |

Don't commit any of these values. They only go into the AWS console and GitHub settings.

---

## 1. Region and a $1 budget alert

1. In the region selector (top right), choose **Asia Pacific (Mumbai) `ap-south-1`**.
   (CloudFront and IAM are global; the region only matters for the bucket.)
2. Open **Billing and Cost Management → Budgets → Create budget**.
3. Choose **Customize (advanced)** → **Cost budget** → Next.
4. Name `breakout-budget`, period **Monthly**, budgeted amount **$1.00** → Next.
5. **Add an alert threshold**: `100` % of budgeted amount, trigger **Actual**, your email.
   Optionally add a second one at `100` % **Forecasted**.
6. Next → **Create budget**.

## 2. Private S3 bucket

1. **S3 → Create bucket**.
2. Bucket type **General purpose**, name `<BUCKET>` (must be globally unique, lowercase).
3. Region **ap-south-1**.
4. Object Ownership: **ACLs disabled**.
5. **Block all public access: ON** (leave all four boxes ticked). CloudFront reads the
   bucket through Origin Access Control, so it never needs to be public.
6. Leave versioning off and default encryption (SSE-S3) as is → **Create bucket**.

## 3. CloudFront distribution

1. **CloudFront → Create distribution**.
   If the wizard asks you to pick a pricing plan, the **Free** plan is enough for this.
2. **Origin domain**: pick the bucket from the list
   (`<BUCKET>.s3.ap-south-1.amazonaws.com`). Don't use the "website endpoint".
3. **Origin access**: **Origin access control settings (recommended)** →
   **Create new OAC** → keep the defaults (Sign requests) → Create.
   If the wizard offers to update the bucket policy for you, accept it.
4. **Viewer protocol policy**: **Redirect HTTP to HTTPS**.
5. Allowed HTTP methods **GET, HEAD**. Cache policy **CachingOptimized**.
   Keep **Compress objects automatically** on (it gzip/brotli-compresses the `.wasm` and `.js`).
6. **Web Application Firewall**: **Do not enable security protections** (WAF costs money).
7. **Default root object**: `index.html`.
8. **Create distribution**.
9. If a banner says the S3 bucket policy needs updating, click **Copy policy**, then
   go to **S3 → `<BUCKET>` → Permissions → Bucket policy → Edit**, paste, **Save**.
   It should look like this (only CloudFront, only this distribution, read-only):

   ```json
   {
     "Version": "2012-10-17",
     "Statement": [
       {
         "Sid": "AllowCloudFrontServicePrincipal",
         "Effect": "Allow",
         "Principal": { "Service": "cloudfront.amazonaws.com" },
         "Action": "s3:GetObject",
         "Resource": "arn:aws:s3:::<BUCKET>/*",
         "Condition": {
           "StringEquals": {
             "AWS:SourceArn": "arn:aws:cloudfront::<ACCOUNT_ID>:distribution/<DISTRIBUTION_ID>"
           }
         }
       }
     ]
   }
   ```
10. Note the distribution's **ID** (`<DISTRIBUTION_ID>`) and **domain name**
    (`dxxxxxxxxxxxx.cloudfront.net`). That domain is the public game link.

## 4. GitHub OIDC identity provider

1. **IAM → Identity providers → Add provider**.
2. Provider type **OpenID Connect**.
3. Provider URL `https://token.actions.githubusercontent.com`.
4. Audience `sts.amazonaws.com` → **Add provider**.

## 5. Deploy role (least privilege)

1. **IAM → Roles → Create role** → trusted entity **Web identity**.
2. Identity provider `token.actions.githubusercontent.com`, audience `sts.amazonaws.com`.
3. GitHub organization `Ayush1860`, repository `Breakout`, branch `main` → Next.
4. Don't attach any managed policy → Next → name it `breakout-github-deploy` → **Create role**.
5. Open the role → **Trust relationships** → check it matches this. Only pushes to
   `main` of this repo (and manual runs on `main`) can assume it:

   ```json
   {
     "Version": "2012-10-17",
     "Statement": [
       {
         "Effect": "Allow",
         "Principal": {
           "Federated": "arn:aws:iam::<ACCOUNT_ID>:oidc-provider/token.actions.githubusercontent.com"
         },
         "Action": "sts:AssumeRoleWithWebIdentity",
         "Condition": {
           "StringEquals": {
             "token.actions.githubusercontent.com:aud": "sts.amazonaws.com",
             "token.actions.githubusercontent.com:sub": "repo:Ayush1860/Breakout:ref:refs/heads/main"
           }
         }
       }
     ]
   }
   ```
6. **Permissions → Add permissions → Create inline policy → JSON**, paste, then name it
   `breakout-deploy`:

   ```json
   {
     "Version": "2012-10-17",
     "Statement": [
       {
         "Sid": "ListBucketForSync",
         "Effect": "Allow",
         "Action": "s3:ListBucket",
         "Resource": "arn:aws:s3:::<BUCKET>"
       },
       {
         "Sid": "WriteSiteFiles",
         "Effect": "Allow",
         "Action": ["s3:PutObject", "s3:DeleteObject"],
         "Resource": "arn:aws:s3:::<BUCKET>/*"
       },
       {
         "Sid": "InvalidateCache",
         "Effect": "Allow",
         "Action": "cloudfront:CreateInvalidation",
         "Resource": "arn:aws:cloudfront::<ACCOUNT_ID>:distribution/<DISTRIBUTION_ID>"
       }
     ]
   }
   ```
7. Copy the role's **ARN** (`arn:aws:iam::<ACCOUNT_ID>:role/breakout-github-deploy`).

## 6. GitHub repository settings

**GitHub → Breakout → Settings → Secrets and variables → Actions.**

| Kind | Name | Value |
|---|---|---|
| Variable | `AWS_REGION` | `ap-south-1` |
| Variable | `S3_BUCKET` | `<BUCKET>` |
| Variable | `CF_DISTRIBUTION_ID` | `<DISTRIBUTION_ID>` |
| **Secret** | `AWS_ROLE_ARN` | the role ARN from step 5 |

The role ARN is a **secret** rather than a variable because the repository is public:
Actions logs of public repos are public, and variables are printed in them, while
secrets are masked. The ARN contains the account ID.

The deploy steps in `.github/workflows/deploy.yml` stay skipped until `S3_BUCKET` is set,
so the workflow is just a build check until then.

## 7. Check it

1. **Actions → Build and deploy → Run workflow** (or push to `main`).
2. All steps should be green, including *Upload to S3* and *Invalidate CloudFront cache*.
3. Open `https://dxxxxxxxxxxxx.cloudfront.net`. The game should load.

### If something goes wrong

| Symptom | Likely cause |
|---|---|
| `Not authorized to perform sts:AssumeRoleWithWebIdentity` | Trust policy `sub` doesn't match `repo:Ayush1860/Breakout:ref:refs/heads/main`, or the OIDC provider audience isn't `sts.amazonaws.com` |
| `AccessDenied` on `s3 sync` | Inline policy bucket name typo, or `ListBucket` on `/*` instead of the bucket ARN |
| CloudFront shows `AccessDenied` XML | Bucket policy from step 3.9 missing, or the origin is the website endpoint instead of the bucket |
| Blank page, console says *Incorrect response MIME type. Expected 'application/wasm'* | `index.wasm` uploaded without `--content-type application/wasm` |
| Old version still showing | Invalidation still running (takes a minute or two); hard-refresh the browser |

## Cost

For a small portfolio game this is effectively free: a few hundred KB in S3, CloudFront's
free tier (or Free plan) covers the traffic, and the first 1,000 invalidation paths a
month are free (each deploy uses one, `/*`). The $1 budget alert from step 1 tells you
if that ever changes.
