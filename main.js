const {app, BrowserWindow, dialog} = require('electron')
const {autoUpdater} = require("electron-updater")

app.whenReady().then(() => {
  const window = new BrowserWindow()
  dialog.showMessageBox(window, {message: app.getVersion()})

  autoUpdater.checkForUpdatesAndNotify()
})

app.on('window-all-closed', () => app.quit())
