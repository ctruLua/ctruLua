# filepicker
## filePicker([workingDirectory[, bindings[, callbacks[, ...]]]])
### Argument: workingDirectory
The directory that shows up first in the file browser.
If this is nil, ctr.fs.getDirectory() is used.
The recommended form is sdmc:/path/ or romfs:/path/, but it can be a simple /path/ instead.
#### Possible values
- string
- nil

### Argument: bindings
A table, list of filetypes and key bindings related to these filetypes.
#### Format
```
{
  __default, __directory, [Lua regexp] = {
    [keys from ctr.hid.keys()] = {
      function
      string
    }...
    __name = string
  }...
}
```

The Lua regexp is matched against the filename to determine if it is of this type.
__directory is the "file type" for directories
__default is the "file type" for files that cannot be matched with any other.
  Also, every other type inherits the values it doesn't define from __default.
A file type contains the human-readable name (__name) of the type, displayed on the bottom screen,
  as well as an optional binding for each key.
The optional binding is formed of an anonymous function, followed by the key's label to be displayed on the bottom screen,
  if the label is nil, the key isn't displayed but still bound.

The function is defined as-is:
##### function(externalConfig, selected, bindingPattern, bindingKey)
externalConfig.workingDirectory is the active directory for filePicker, doesn't necessarily match ctrµLua's.
externalConfig.bindings, externalConfig.callbacks and externalConfig.additionalArguments all are the arguments passed to filePicker,
  starting from position 2.
externalConfig.fileList is the list of files currently displayed by filePicker, in the same format as is returned by ctr.fs.listDirectory().
selected.inList is the absolute position of the cursor in externalConfig.fileList
selected.offset is the number of items skipped for display from fileList
bindingPattern is the [Lua regexp] defined earlier, and bindingKey the [key] that triggered this event.
This function may return the same thing as filePicker itself (Defined later here), or nothing. If it returns nothing,
filePicker will keep running, if it returns the same returns as filePicker, filePicker will exit, returning these values.

#### Notes
Sane defaults are set if you did not set them otherwise:
__default.x quits filePicker, returning the current directory, "__directory", nil and "x". See the returns to understand what this means.
__directory.a changes directories to that directory.

### Argument: callbacks
A table defining the callbacks ran at the end of each of the equivalent phases.
#### Format
```      
{
  drawTop, drawBottom, eventHandler = function
}
```
All of these take the following parameters: (externalConfig, selected)
They have the meaning defined earlier.

#### Notes
Although drawTop and drawBottom are ran at the end of their respective functions,
eventHandler is not, as it cannot be without being run repeatedly, so it's run at the beginning of
the ACTUAL eventHandler instead of its end.

### Argument: ...
Additional parameters. All of these are aggregated orderly in externalConfig.additionalArguments and passed around as explained earlier.
They have no specific meaning unless defined so by event handling functions.

### Return 1: selectedPath
The path selected by the either, may or may not have the sdmc:/romfs: prefix, depending on your input.
A string.

### Return 2: bindingPattern
The pattern the file matched to. You can use this to know exactly which kind of file you're dealing with
A string.

### Return 3: mode
Included handlers may have it be "open", in case you're opening an existing file, "new" in case the user wants to create a new file,
Or nil. A "nil" is assumed to mean that the user didn't pick anything.
A string or nil.

### Return 4: key
The key that triggered the event that made filePicker exit.
A string.

## Included event handlers you have available are:
- filepicker.changeDirectory - The name is on the tin, change workingDirectory to the active element and refresh the file list for that path.
- filepicker.openFile - Quits and returns as described by this document based on the active element.
- filepicker.newFile - Prompts the user to input a file name manually, relative to the current working directory, and with FAT-incompatible characters excluded. Quits and returns that.
- filepicker.nothing - Do nothing and keep on running. Literally. This is used as a plug to enable an action for all (or a certain type) but files of a certain type (or more precise than the initial type).