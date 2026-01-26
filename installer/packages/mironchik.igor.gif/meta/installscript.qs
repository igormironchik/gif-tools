
function Component()
{
}

Component.prototype.createOperations = function()
{
	component.createOperations();

	if( installer.value( "os" ) === "win" )
	{
		component.addOperation( "Execute", "{0,1638}", "@TargetDir@\\bin\\VC_redist.x64.exe",
			"/silent" );

		component.addOperation( "CreateShortcut", "@TargetDir@\\bin\\gif-editor.exe",
			"@StartMenuDir@\\GIF Editor.lnk", "workingDirectory=@TargetDir@\\bin" );
			
		component.addOperation( "CreateShortcut", "@TargetDir@\\bin\\gif-recorder.exe",
			"@StartMenuDir@\\GIF Recorder.lnk", "workingDirectory=@TargetDir@\\bin" );

	}
}
