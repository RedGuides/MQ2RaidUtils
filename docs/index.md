---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2raidutils.177/"
support_link: "https://www.redguides.com/community/threads/mq2raidutils.66877/"
repository: "https://github.com/RedGuides/MQ2RaidUtils"
config: "MQ2RaidUtils.ini"
authors: "dewey2461"
tagline: "Makes leading raids a little easier by adding an \"All\" option dz/tasks add and remove commands."
acknowledgements: "Sorcier and the RaidManager macro"
---

# MQ2RaidUtils

<!--desc-start-->
This plugin is to make leading raids a little easier by adding an "All" option dz/tasks add and remove commands.

The secondary feature of this plugin is to take raid attendance.

For Non-Raid leaders the option to automatically perform some related tasks such as joining chat channels or removing anon / role may be added later
<!--desc-end-->

## Commands

<a href="cmd-dzadd/">
{% 
  include-markdown "projects/mq2raidutils/cmd-dzadd.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2raidutils/cmd-dzadd.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2raidutils/cmd-dzadd.md') }}

<a href="cmd-dzremove/">
{% 
  include-markdown "projects/mq2raidutils/cmd-dzremove.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2raidutils/cmd-dzremove.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2raidutils/cmd-dzremove.md') }}

<a href="cmd-dzshow/">
{% 
  include-markdown "projects/mq2raidutils/cmd-dzshow.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2raidutils/cmd-dzshow.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2raidutils/cmd-dzshow.md') }}

<a href="cmd-raidtools/">
{% 
  include-markdown "projects/mq2raidutils/cmd-raidtools.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2raidutils/cmd-raidtools.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2raidutils/cmd-raidtools.md') }}

<a href="cmd-taskadd/">
{% 
  include-markdown "projects/mq2raidutils/cmd-taskadd.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2raidutils/cmd-taskadd.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2raidutils/cmd-taskadd.md') }}

<a href="cmd-taskremove/">
{% 
  include-markdown "projects/mq2raidutils/cmd-taskremove.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2raidutils/cmd-taskremove.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2raidutils/cmd-taskremove.md') }}

<a href="cmd-taskshow/">
{% 
  include-markdown "projects/mq2raidutils/cmd-taskshow.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2raidutils/cmd-taskshow.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2raidutils/cmd-taskshow.md') }}

## Settings

Example MQ2RaidUtils.ini,

```ini
[Voxvox]
LogIsON=1
LogEvery=30
LogFileFormat=%Q%G%y%b%a%H%M
LogTimes=Mon 20:00-23:00 | Tue 20:00-02:00 | Sun 11:00-15:00
```

For more information on these settings, refer to [/raidtools](cmd-raidtools.md)
