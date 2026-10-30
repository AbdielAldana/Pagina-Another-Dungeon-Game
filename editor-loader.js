// This is a classic .js entry point so ordinary static hosts can start the editor.
// Catch module/MIME errors before the application has a chance to initialize.
(() => {
  const banner = document.getElementById('load-banner');
  const loading = document.getElementById('canvas-loading');
  const scriptURL = new URL(document.currentScript.src);

  function showError(message) {
    banner.hidden = false;
    banner.classList.add('error');
    banner.textContent = message;
    loading.hidden = false;
    loading.textContent = 'No se pudo iniciar el editor. Revisa el aviso de arriba.';
  }

  if (location.protocol === 'file:') {
    showError('Abre esta página desde un servidor web o Live Server de VS Code para cargar el editor.');
    return;
  }

  const watchdog = setTimeout(() => {
    showError('La carga del editor está tardando demasiado. Comprueba tu conexión y vuelve a cargar la página.');
  }, 25000);

  const moduleURL = new URL('editor.js', scriptURL);
  moduleURL.search = scriptURL.search;
  import(moduleURL.href).then(() => {
    clearTimeout(watchdog);
  }).catch(error => {
    clearTimeout(watchdog);
    console.error('No se pudo iniciar el taller de cuartos:', error);
    showError(`No se pudo iniciar el editor. Vuelve a subir editor.html, editor-loader.js, editor.js, editor-core.js y editor-game-format.js a la misma carpeta. Detalle: ${error.message}`);
  });
})();
