---
tags:
  - command
---

# /raidtools

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/raidtools [help|dump] | log [on|off|File <name>|Times <str>|Every <interval>]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Configures raidtools logging, dumps a log of all players in raid, and shows help text
<!--cmd-desc-end-->

## Options

| Option | Description |
|--------|-------------|
| `help` | show help text |
| `dump` | Records a list of players currently in the raid |
| `log [on|off]` | Turns auto logging on and off |
| `log File <name>` | The File "name" will be parsed using strftime. The most common parameters are:<br><br>**%a** Abbreviated weekday name<br>**%b** Abbreviated month name<br>**%d** Day of month as decimal number (01 ? 31)<br>**%H** Hour in 24-hour format (00 ? 23)<br>**%m** Month as decimal number (01 ? 12)<br>**%M** Minute as decimal number (00 ? 59)<br>**%S** Seconds as decimal number (00 ? 61)<br>**%Y** Year with century, as decimal number<br>**%%** Percent sign<br><br>**The name can also have the following**<br>**%G** Guild name<br>**%V** Server name<br>**%C** Character name<br>**%P** Everquest path<br>**%Q** Macroquest path |
| `log Times <str>` | Describes when to log. <br>example: Mon 20:00-23:00 &#124; Tue 20:00-02:00 |
| `log Every <min>` | The log interval is (1-60) and represents every N minutes to perform a raid dump. |
