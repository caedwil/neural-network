var {app, BrowserWindow, Menu} = require('electron');

let win;

function createWindow() {
  win = new BrowserWindow({
    width: 800,
    height: 600,
    autoHideMenuBar: true
  });
  win.loadURL(`file://${__dirname}/pages/index.html`);
  win.on('closed', () => {
    win = null;
  });
}

function createMenu() {
  // Create menu template.
  // View
  //   -> Refresh
  //   -> Developer Console
  var template = [{
    label: 'View',
    submenu: [{
      label: 'Refresh',
      accelerator: 'F5',
      click: function(item, focusedWindow) {
        // If window to refresh is the main window, close all other
        // windows.
        if (focusedWindow.id === 1) {
          BrowserWindow.getAllWindows().forEach((win) => {
            if (win.id > 1) win.close();
          });
        }
        focusedWindow.reload();
      }
    }, {
      label: 'Developer Console',
      accelerator: 'F12',
      click: function(item, focusedWindow) {
        focusedWindow.webContents.toggleDevTools();
      }
    }]
  }];
  Menu.setApplicationMenu(Menu.buildFromTemplate(template));
}

app.on('ready', () => {
  createWindow();
  createMenu();
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('active', () => {
  if (win === null) createWindow();
});
