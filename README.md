<h1>typing</h1>
<p>Speed typing test program to train and practice touch typing, made in C</p>
<p>This program reads the "config.conf" file that is to be read from the ~/.config/typing/ file</p>
<p>Currently this app is for linux only and can only read files given to it by the user through the "file_path" keyword in the config by specifying the absolute path to the text file.</p>
<p>Available configs are :</p>
<ul>
  <li>lines : maximum liens to print</li>
  <li>cursor_color : the color of the cursor</li>
  <li>error_color : the color if a mistake is made</li>
  <li>transparent : will the previewed text be transparent or not</li>
  <li>file_path : path to the text file to be typed out</li>
  <li>default_mode : either simple(No timer) or timed</li>
  <li>time : if the mode is timed, this is the time in seconds</li>
  <li>preview : preview the the text file before starting</li>
  <li>live_mistakes : show a highlight in the color specified by the error_color keyword while writing if a mistake is made</li>
  <li>print_mistakes : show all of the mistakes made at the end.</li>
</ul>
<p>You can use the text files in the simples folder to start, or give your own file</p>
<p>The simples file was sto..borrowed from the monkeytype web app repository : https://github.com/monkeytypegame/monkeytype</p>
