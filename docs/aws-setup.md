# AWS setup (one-time)

The WebAssembly build is hosted on **AWS Amplify Hosting** as a manually deployed app (not
connected to the repo). GitHub Actions builds the game and deploys it with short-lived
credentials from an **IAM role** that it assumes through **OIDC**. No long-lived AWS keys
exist anywhere.

```
GitHub (push to main) → Actions: build with Emscripten, zip index.html/.js/.wasm
                      → assume IAM role via OIDC (no stored keys)
                      → amplify create-deployment → upload zip → start-deployment → poll get-job
Player → https://main.<APP_ID>.amplifyapp.com → Amplify Hosting (HTTPS + CDN)
```

All commands below use the AWS CLI with region `ap-south-1`. Replace:

| Placeholder | Example | Where you get it |
|---|---|---|
| `<ACCOUNT_ID>` | `123456789012` | `aws sts get-caller-identity` |
| `<APP_ID>` | `d1abc2def3ghij` | Output of step 2 |
| `<EMAIL>` | you@example.com | Your email for the budget alert |

The repository never contains the account ID or the role ARN. They only go into AWS and
GitHub settings.

---

## 1. Check you're in the right account

```bash
aws sts get-caller-identity
```

Prefer an IAM Identity Center (SSO) profile over the root user for this.

## 2. Amplify app and branch

```bash
aws amplify create-app --name breakout --platform WEB --region ap-south-1
aws amplify create-branch --app-id <APP_ID> --branch-name main --region ap-south-1
```

`create-app` prints the `appId`. The branch's public URL is `https://main.<APP_ID>.amplifyapp.com`,
and it starts serving after the first deployment.

## 3. GitHub OIDC identity provider

Skip this step if `aws iam list-open-id-connect-providers` already lists
`token.actions.githubusercontent.com`. One provider per account is shared by all roles.

```bash
aws iam create-open-id-connect-provider \
  --url https://token.actions.githubusercontent.com \
  --client-id-list sts.amazonaws.com
```

## 4. Deploy role (least privilege)

First get the exact OIDC subject GitHub will send. This repository uses GitHub's
**immutable subject** format, in which the owner and repository carry their permanent
numeric IDs (`repo:<owner>@<owner_id>/<repo>@<repo_id>:...`):

```bash
gh api repos/Ayush1860/Breakout/actions/oidc/customization/sub
# → "use_immutable_subject": true, "sub_claim_prefix": "repo:Ayush1860@89765953/Breakout@1470315602"
```

The trust policy must match that prefix plus `:ref:refs/heads/main`. With IDs in the
subject, the role stays tied to this exact repository, even if it is renamed or a new repo
with the same name is created later. (Repos without immutable subjects send
`repo:Ayush1860/Breakout:ref:refs/heads/main` instead.)

`trust.json`: only pushes to `main` of this repository (and manual runs on `main`) can
assume the role.

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
          "token.actions.githubusercontent.com:sub": "repo:Ayush1860@89765953/Breakout@1470315602:ref:refs/heads/main"
        }
      }
    }
  ]
}
```

`deploy-policy.json`: the role can only create, start and check deployments of this one app.

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": ["amplify:CreateDeployment", "amplify:StartDeployment", "amplify:GetJob"],
      "Resource": "arn:aws:amplify:ap-south-1:<ACCOUNT_ID>:apps/<APP_ID>/*"
    }
  ]
}
```

```bash
aws iam create-role --role-name breakout-github-deploy \
  --assume-role-policy-document file://trust.json
aws iam put-role-policy --role-name breakout-github-deploy \
  --policy-name breakout-amplify-deploy --policy-document file://deploy-policy.json
```

Optional check that the policy allows nothing else:

```bash
aws iam simulate-principal-policy \
  --policy-source-arn arn:aws:iam::<ACCOUNT_ID>:role/breakout-github-deploy \
  --action-names amplify:CreateDeployment amplify:DeleteApp s3:PutObject \
  --resource-arns arn:aws:amplify:ap-south-1:<ACCOUNT_ID>:apps/<APP_ID>/branches/main
# → CreateDeployment: allowed, everything else: implicitDeny
```

## 5. Zero-spend budget for Amplify

The budget is filtered to the AWS Amplify service so other projects in the account don't
trigger it. It emails you as soon as Amplify costs anything (over $0.01 in a month).

```bash
aws budgets create-budget --account-id <ACCOUNT_ID> \
  --budget '{"BudgetName":"breakout-amplify-zero-spend","BudgetType":"COST","TimeUnit":"MONTHLY",
             "BudgetLimit":{"Amount":"1.00","Unit":"USD"},"CostFilters":{"Service":["AWS Amplify"]}}' \
  --notifications-with-subscribers '[{"Notification":{"NotificationType":"ACTUAL","ComparisonOperator":"GREATER_THAN",
             "Threshold":0.01,"ThresholdType":"ABSOLUTE_VALUE"},
             "Subscribers":[{"SubscriptionType":"EMAIL","Address":"<EMAIL>"}]}]'
```

## 6. GitHub repository settings

```bash
gh variable set AMPLIFY_APP_ID --body <APP_ID>
gh variable set AWS_REGION --body ap-south-1
aws iam get-role --role-name breakout-github-deploy --query Role.Arn --output text | gh secret set AWS_ROLE_ARN
```

| Kind | Name | Value |
|---|---|---|
| Variable | `AMPLIFY_APP_ID` | `<APP_ID>` |
| Variable | `AWS_REGION` | `ap-south-1` |
| **Secret** | `AWS_ROLE_ARN` | the role ARN from step 4 |

The role ARN is a **secret** because the repository is public. Actions logs of public repos
are public, and variables are printed in them, while secrets are masked. The ARN contains
the account ID.

The deploy steps in `.github/workflows/deploy.yml` are skipped until `AMPLIFY_APP_ID` is set,
so the workflow is just a build check until then.

## 7. Check it

1. **Actions → Build and deploy → Run workflow** (or push to `main`). The *Deploy to Amplify
   Hosting* step should end with `status: SUCCEED`.
2. Check the site:
   ```bash
   curl -sI https://main.<APP_ID>.amplifyapp.com/ | head -1                                  # HTTP/2 200
   curl -sI https://main.<APP_ID>.amplifyapp.com/index.wasm | grep -i content-type           # application/wasm
   ```

### If something goes wrong

| Symptom | Likely cause |
|---|---|
| `Not authorized to perform sts:AssumeRoleWithWebIdentity` | Trust policy `sub` doesn't match what GitHub sent: the immutable form `repo:Ayush1860@89765953/Breakout@1470315602:ref:refs/heads/main` versus the legacy form, or the workflow ran from another branch. CloudTrail's failed `AssumeRoleWithWebIdentity` event shows the exact `sub` in `userIdentity.userName`. |
| `AccessDeniedException` on `amplify:CreateDeployment` | Wrong app ID in the policy resource, or `AMPLIFY_APP_ID` doesn't match the app |
| `curl` upload fails with 403 | The pre-signed upload URL expired; re-run the workflow |
| Job ends `FAILED` | The zip doesn't have `index.html` at its root; check the *Package site* step's `unzip -l` listing |
| Old version still showing | Hard-refresh the browser; Amplify switches to the new version when the job succeeds |

## Cost

For a small portfolio game this is effectively free. Manual deployments use no build
minutes, and storage and data served stay within Amplify's free tier. The zero-spend budget
from step 5 emails you if that ever changes.
