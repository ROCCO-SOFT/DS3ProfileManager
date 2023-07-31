NECCO EDIT(Alpha) Release Notes 


1. About this app 

  This app is a video editing application that runs on Windows. 
  You can easily trim or combine unnecessary parts of MP4 files shot with smartphones or digital cameras, synthesize audio files and image files, and synthesize text characters. 
  The completed project can be output as an MP4 file, so it can be posted on video sites such as YouTube. 
  Since it is still under development, the functions are extremely scarce, but we aim to make it possible for anyone to easily enjoy full-fledged video editing. 


2. Operating environment 

  OS Windows 10 64bit or later 
  Memory 8GByte or more 
  HDD free space 100GByte or more 
  This application consumes a lot of memory. I think the more memory you have, the better. 


3. Install 

  DirectX9 is required to run NECCO EDIT. Please download and install DirectX9 from the following Microsoft website in advance. 
  https://www.microsoft.com/en-us/download/details.aspx?id=35 

  Execute NeccoEdit Setup.msi. Please follow the instructions on the screen to install. 


4. How to operate

  For details, please refer to the included NECCO EDIT user manual ManualJPN.pdf (under preparation). 


5. History (newest order) 

  0.1.241 (2023/07/03)
    - Corresponds to the waveform display of video clips and audio clips.
    - Measures for the problem of abnormal termination when undo redo is performed during title editing.
    - Changed the upper limit of audio level from 1.2 to 1.0.
    - Fixed an incorrect ruler display at 24fps.
    - Improved ruler display when clip trimming.
    - Fixed a bug that the scroll bar was not updated after changing the scale of the timeline.
    - Fixed the problem that display garbage remains when audio level is operated.
    - Shuttle stops by pressing the mouse wheel.

  0.1.235 (2023/06/26)
    - Added the figure drawing function to the title. 
    - A rotation function has been added to title text, still images, and figures. 
    - Improved rubber band operation in image quality adjustment and preview in title function. 
    - Improved the operation of the snap function of the timeline. 
    - Fixed the problem that the resolution is lowered when exporting when low load mode is enabled. 

  0.1.223 (2023/06/21)
    - Fixed the user interface of the title still image function. 

  0.1.219 (2023/06/19)
    - When dragging on the timeline, it snaps to the edit point. 
    - Added low load playback mode. Select [Edit]-[Preferences]-[Low load mode]. 
    - You can now rotate the still image function of the title. 
    - Added the provisional version of the Japanese manual. 

  0.1.205 (2023/06/13)
    - Supported some alpha channels. Clips with alpha and transitions between titles can now be applied.
    - Supported 24p (23.98fps) in project settings. 
    - Fixed a bug that the MP4 output destination folder was not memorized.
    - Added a function to maintain the audio level of the key in the audio level track. Right-click the appropriate audio level key and click Keep. 
    - Fixed so that the frame position can also be changed when dragging the audio level key. 
    - Fixed so that [Adjust Image] works even when a still image clip is placed on a track. 
    - Fixed so that alpha channel (transparency) is also evaluated when still image clips are placed on the track. 
    -Fixed a bug that could not be processed correctly when the file name or path contained machine-dependent characters. 

  0.1.193 (2023/06/01)
    - Fixed a bug where only half of the timeline could be played back when 60fps was selected in the project settings. 
    - Fixed a bug where initial effect presets could not be loaded correctly. 
    - Added support for videos with a different aspect ratio from the project settings. 
    - Fixed a bug that caused an access violation when exiting the application with some property dialogs displayed. 
    - Fixed a bug that caused an access violation when closing the application when opening a specific clip. 
    - Changed to display the image of the in point for clips that have been trimmed in the thumbnail of the bin window.
    - Effects such as titles and effects are now reflected in thumbnails in the bin window. 
    - You can now right-click a clip on the timeline and select image quality settings to adjust the layout and image quality.
    - The status indicator at the bottom of the window now shows the playback time in addition to the current position on the timeline. 
    - Fixed a bug that caused the application to hang when pts was seeking discontinuous MP4 clips. 
    - Fixed a bug that caused the application to terminate with an access violation if the project settings were made while the timeline was playing. 
    - Supported the resolution of Youtube shorts. Choose 1080x1920 from the project settings. 
    - Equipped with voice level function. Right-click the track header and click [Show Audio Level]. 
    - Windows older than Windows 10 are not supported. 

  0.1.187 (2023/02/08)
    - Fixed a bug where transitions could not be set correctly when track synchronization was enabled. 
    - Fixed a bug that the field order was incorrect when closing the project setting dialog. 

  0.1.185 (2022/08/15)
    - Provisional support for H.264 output on NVIDIA(R) NVENC(TM). 

  0.1.181 (2020/04/25) 
    - Backup projects are retained for up to 30 days. 

  0.1.179 (2020/01/01)
    - You can now specify the type of transition that will be applied by default. 
    - Fixed a bug where incorrect menu items were displayed in the right-click menu on the preview when editing titles. 
    - Fixed a bug that caused high CPU load while displaying effect settings, transition settings, mixer settings dialogs, etc. 
    - Added wipe, push slide, and push stretch to basic transition. Also added presets. 
    - Rotate effect has been changed to layout effect. In addition, the screen position, size, and crop settings have been added. 
    - Added support for animated GIF. 
    - Fixed other minor bugs. 

  0.1.171 (2020/01/01)
    - Fixed a bug where clips could not be read correctly on Windows 10. 

  0.1.163 (2019/07/13)
    - Changed so that video of all tracks is output even during title editing. 
    - Fixed a bug where the project file name was not restored to the original name even when opening an auto-saved project. 

  0.1.159 (2019/07/03)
    - Supported clip fade-in/fade-out.
    - [All clips]-[Fade in] and [All clips]-[Fade out] have been added to the right-click menu of the track header on the timeline. 
    - Improved the movement of clip copy/paste in the timeline.
    - Fixed a bug where gaps were sometimes created between clips when batch setting transitions (right-click track header [All clips] - [Transitions]).
    - Added a function to preview and play clips in the bin window. Double-click the clip or click Play in Preview on the right-click menu of the mouse. 
    - Fixed a bug where gaps were sometimes created between clips in the batch transition function of tracks. 
    - Fixed a bug where the timeline cursor would go out of the screen when the timeline scale was changed. 
    - Fixed an issue where the entire timeline would shift if a transition was applied to a clip when main track sync was enabled ([Edit]-[Settings]-[Sync to main track]). 
    - Fixed a bug where even if a clip with fade-in or fade-out on the main track was deleted, the following clip would not shift to the left. 

  0.1.147 (2019/06/26)
    - Added a preset function to the title function. 
    - Fixed so that the file name is the same as the project when exporting still images. 
    - Added a function to save clip information to a file. Use when you want to reuse clips in other projects. 
    - Fixed the problem that the application becomes unresponsive when exporting MP4 files that are in use by other applications. 

  0.1.121 (2019/06/18)
    - If a file referenced by the project has been moved or renamed, an inquiry will be made when the project is opened again. 
    - Picture-in-picture has been added to the Track Mixer. 
    - Added Go to Start and Go to End buttons in the playbar. 
    - [Set transitions for all clips] and [Delete all clips] have been added to the track right-click menu on the timeline. 
    - It is now possible to operate the timeline while displaying the effect, transition, and mixer setting windows such as the image quality setting window. 
    - Added property setting to show transition blocks in audio tracks. 
    - The text is selected in the initial state in the title setting window. 
    - Added an option to restore the project when restarting after an abnormal termination of the application. 
    - Fixed a bug that very rarely caused the application to crash while seeking on the timeline. 
    - Fixed the problem that the color of the text in the title may change when outputting to a file. 
    - Fixed an issue where, when a very short clip was at the beginning of a track, if you tried to set a transition near it, it would be set in an invalid state. 
    - In the title setting window, recently used fonts are displayed preferentially at the top of the list.
    - Added [Rejoin clip] to the clip menu to rejoin a clip that was once split into a single clip. 
    - Fixed a bug that caused the application to crash when loading a still image file with transparency.
    - Fixed a bug that caused a malfunction when reading a project that included still image files in the timeline and trimming the Out point of the still image clip. 
    - Fixed so that the timeline scale information is also saved in the project. 
    - Fixed an issue where items in the foreground could not be clicked when items overlapped on the title edit screen. 
    - Fixed so that newly added items are brought to the front on the title edit screen. 
    - Added a setting to drop frames when there is not enough time ([Edit]-[Settings]-[Allow frame drop]). 
    - Added transparency setting to the still image of the title function. 

  0.1.77 (2019/06/06)
    - Added rotate to clip effect. 
    - Added color adjustment to clip effect. 
    - Added slide, stretch and split transitions. 
    - Added alpha transition to transition. 
    - Only the first half of the clip is selected after splitting the clip. 
    - When adding a clip on the right menu of the track, it has been changed so that it is placed at the position of the timeline cursor.
    - Fixed the problem that still images could not be read correctly when opening a project file containing still images. 
    - Fixed the problem that the application may crash when repeatedly stopping playback.
    - Fixed a bug that malfunctioned when selecting [In point] or [Out point] from the right-click menu of the clip in the transition section. 
    - Fixed a bug that the settings were not restored even if the clip properties were closed with [Cancel]. 
    - Fixed a bug that could not be dragged when the split of the main window was out of the client area. 
    - The extension of the project file has been changed. Please open the project file saved in 0.1.33 with [Open] in the [File] menu. 
    - Added support for saving a project as a template. 

 0.1.33 (2019/05/16)
    - Alpha Version Release 


6. Disclaimer 

  The author cannot be held responsible for any damages caused by using this software. 
  Please use it at your own risk. 
  The images shown in the manual, etc. are under development. Subject to change without notice. 


7. About reprinting 
  You may attach it to magazines and redistribute it freely 

  on the condition that you do not change the application, attached documents, icons and data . 
  If you don't mind, I would appreciate it if you could contact me at the following.
  neccoedit@gmail.com 


8. Trademarks and Licenses
 
  Windows, Windows10 and Windows11, DirectX are trademarks of Microsoft.
  FFmpeg is a trademark of Fabrice Bellard, the founder of the FFmpeg project. 
  Intel, QuickSyncVideo are trademarks of Intel. 
  NVIDIA and NVENC are trademarks of NVIDIA. 
  OpenCV is a trademark of the OpenCV team. 
  OpenH264 is a trademark of Cisco Systems. 

  NECCO EDIT uses the LGPLv3.0 FFmpeg project library 
  https://www.gnu.org/licenses/lgpl-3.0.html 
  If you need the FFmpeg source code included in NECCO EDIT, Please contact us at neccoedit@gmail.com. 


Copyright(C) 2019-2023 ROCCO SOFT.