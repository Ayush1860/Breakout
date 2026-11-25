// Runs before the Emscripten runtime starts (linked with --pre-js).
// This lives here rather than inline in shell.html so the page has no inline
// scripts and can be served with a strict Content-Security-Policy.

Module['canvas'] = document.getElementById('canvas');

// No right-click menu over the game.
Module['canvas'].addEventListener('contextmenu', function (e) {
    e.preventDefault();
});

// Game keys shouldn't scroll the page when the window is small.
window.addEventListener('keydown', function (e) {
    if ([' ', 'ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown'].indexOf(e.key) !== -1) {
        e.preventDefault();
    }
});
