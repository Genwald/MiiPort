# MiiPort
A Nintendo Switch homebrew for importing and exporting Miis.
## Installation
[Download a release](https://github.com/Genwald/MiiPort/releases/latest) and then place the .nro file at `sd:/switch/MiiPort.nro`


Some features require the setting 
```
[mii]
is_db_test_mode_enabled=u8!0x1
```
which can be set in `/atmosphere/config/system_settings.ini`
## Screenshots
<img alt="Import tab" src="https://user-images.githubusercontent.com/11589515/116328811-6dd88380-a78f-11eb-841d-b06d5ed3f587.jpg" width="65%">
<img alt="Export tab"  src="https://user-images.githubusercontent.com/11589515/116329846-d163b080-a791-11eb-917f-1d4921a54545.jpg" width="65%">


## Usage
Place Mii character files in `sd:/MiiPort/Miis/` with a file extension that corresponds to their format.  
currently supported import formats include:
- jpeg images of Mii QR codes
    - requires the mii QR key
- charinfo
    - can also use the `.bin` extension
- NFIF
    - can also use the `.dat` extension
- coredata
- storedata

## QR key
In order to import Miis from a qr code, you must supply the Mii QR key. This is needed to decrypt the Mii data stored in Mii QR codes. You can find this on the internet by searching for "Mii QR key".  
This program looks for the key in hex in the file `/MiiPort/qrkey.txt`.
It will accept it in a variety of formats such as:
`[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]`
or `AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA`
