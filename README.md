# imgterm
Display images in the terminal.

Enhance the resolution by mapping 4x8 pixel cells to different Unicode characters. Inspired by [TerminalImageViewer](https://github.com/stefanhaustein/TerminalImageViewer), but using a slightly different algorithm:

For each Unicode character:

1. Calculate the average color of the foreground and background.
2. Calculate the distance between the cell using the average colors and the original cell.

Find the Unicode character with the closest distance to the original cell and apply the calculated average foreground and background color of it.


## Usage
```
Usage: imgterm [options] [files]
Options
    -w width
        Pixel width of the image. Use screen width if set to 0
    -h height
        Pixel height of the image. Use screen height if set to 0
    -p percentage
        Percentage of the image height to the screen height (Default=50)
    -e level
        Enhance level (Default=2)
        0 = Use space
        1 = Use lower half block
        2 = Use more unicode characters
    -r  Use the raw size of the image
    -8  Use 8-bit colors
    -?  Print this help
```


## Installation
```
git clone https://github.com/evanlin96069/imgterm.git
cd imgterm
make && sudo make install
```
