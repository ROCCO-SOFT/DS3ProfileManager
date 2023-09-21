DARK SOULS REMASTERED / DARK SOULS III / ELDEN RING / SEKIRO
Profile Manager (Alpha) Release Notes


  Thank you for downloading this application.
  If you view this release note in Notepad, check [Format]-[Wrap at right edge] to make it easier to read.
  This release note is common to DARK SOULS REMASTERED (DSR) / DARK SOULS III (DS3) / ELDEN RING (ER) / SEKIRO (SKR).
  Replace {DSR|DS3|ER|SKR} in the text with the keyword corresponding to each game.
  Example: When using with DARK SOULS III, replace {DSR|DS3|ER|SKR}ProfileManager.msi with DS3ProfileManager.msi.


1. About this app

  This application is an application that allows you to save the save data of the STEAM version DSR / DS3 / ER / SKR to the HDD and return it to the game with a simple operation. is.
  Save your save data frequently during gameplay to prepare for unforeseen circumstances.
  You can save multiple save data, so you can create data for Boss Rush.


2. Operating environment

  OS Windows 10, 11 64bit or later
  Memory 8GByte or more
  HDD free space 1GByte or more
  STEAM version DSR / DS3 / ER / SKR must be installed.


3. Installation

  Run {DSR|DS3|ER|SKR}ProfileManager.msi. Please follow the instructions on the screen to install.
  *The message "Windows has protected your PC" may be displayed. In that case, click [Details] and then click the [Run] button.
  *To start directly without using the installer, run {DSR|DS3|ER|SKR}ProfileManager.exe.

 
4. How to operate

  a. To start {DSR|DS3|ER|SKR}Profile Manager
     Click [Roccosoft] - [{DSR|DS3|ER|SKR} Profile Manager] from the Windows Start menu.
     *To start directly without using the installer, run {DSR|DS3|ER|SKR}ProfileManager.exe.

  b. To save save data while playing the game
     Click the [Save] button at the bottom of the window. *
     Saved data will be displayed in the list on the window.
     *If you are playing the game, select [Exit Game] from the game system screen to return to the main screen.

  c. To return the saved save data to the game
     With the save data selected in the list, click the [Load] button. The data currently being played will be overwritten, so please exercise caution.
     *If you are playing the game, select [Exit Game] from the game system screen to return to the main screen.

  d. To change the name of saved save data
     Click the name of the save data to enter label edit mode and enter a new name.
     *The following characters cannot be used in the name. \ / : * ? " < > |

  e. To overwrite the save data you are playing with the save data that has already been saved
     With the saved data in the list selected, click the [Overwrite] button.
     *If you are playing the game, select [Exit Game] from the game system screen to return to the main screen.

  f. To delete save data
     With the saved data in the list selected, click the [Delete] button.

  g. To disable autosave
     Right-click the title and click [Enable Autosave] to remove the check.

  h. Sort function
     You can drag each item up or down to rearrange them in any order (user specified order).
     Clicking the column header (Name, Date only) in the list switches between ascending order, descending order, and user-specified order.

  j. Changing the color of the list part (tentative)
     You can change the color of the list part by changing the following value (RGB value) of the registry key.
     *If you do not have knowledge about registry operations, please do not perform this operation as it may damage the system.
     [KEY_CURRENT_USER\SOFTWARE\RoccoSoft\{DSR|DS3|ER|SKR}ProfileManager\Settings]
     "ProfileList.TextColor"=dword:00000000
     "ProfileList.TextBkColor"=dword:00ffffff
     "ProfileList.BkColor"=dword:00ffffff

  k. About save data storage location and registry key location
     Save data is saved in the following folder.
     C:\Users\<username>\Documents\{DSR|DS3|ER|SKR}ProfileManager
     The registry keys related to settings are as follows.
     [KEY_CURRENT_USER\SOFTWARE\RoccoSoft\{DSR|DS3|ER|SKR}ProfileManager\Settings]


5. History (newest first)

 0.1.7 (2023/09/21)
    - Fixed a bug where the window would close when pressing the ESC or ENTER key.
    - The font color and background color of the list part can now be changed.

 0.1.5 (2023/08/27)
    - Moved the location of the start menu under Roccosoft.
    - Added auto save function.
    - Added sorting function.
    - Added support for DARK SOULS REMASTERED.

 0.0.3 (2023/08/11)
    - Fixed a bug that the app crashes after saving

 0.0.2 (2023/08/01)
    - Changed profile ID management method

 0.0.1 (2023/07/31)
    - First release


6. Disclaimer

  The author cannot be held responsible for any damage caused by using this software.
  Please use it at your own risk.


7. Reproduction

  Provided that you do not modify the application, accompanying documents, icons or data.
  You are free to attach it to magazines or redistribute it on your website.
  If you don't mind, I would appreciate it if you could contact me at the following.
  neccoedit@gmail.com


8. Trademarks; Licenses

  Windows, Windows10 and Windows11, DirectX are trademarks of Microsoft.
  DARK SOULS, DARK SOULS REMASTERED, DARK SOULS III, ELDEN RING, SEKIRO and ARMORED CORE VI are trademarks of FromSoftware and Bandai Namco.
  STEAM is a trademark of Valve Corporation.


Copyright(C) 2023 ROCCO SOFT.
