## MacBook Charge Limiter Tool

MacBook Charge Limiter Command-Line Tool.

This `macbook-charge-limiter` tool allows MacBook users to set a maximum
battery charge level, especially useful for those who leave their laptops
plugged in most of the time.


### Purpose / Why?

Modern electronics do a very good job of battery charge management.
Nonetheless...

Lithium Ion batteries do best when kept at charge level between ~ 30%-80%.

Lithium Ion batteries held at or close to 100% for extended periods of time
*may* begin to deposit metallic lithium on their anode, oxidize at the cathode,
cause permanent charge capacity degradation, and even lose stability,
produce carbon dioxide (CO2), and begin to swell.

This may initially be detected as altered feel (shorter travel) on the 
laptop's space bar and trackpad clicks, and ultimately as swelling of the
case itself and significant interference with those keys and clicks.

This tool is designed for advanced users to address some of these concerns.


### Project Goal / Philosophy

This code intends to be as clear, simple, and short as possible to allow for
easy audit, build, and trust. 

The original code was forked from theopolis's tool `smc-fuzzer`, 
which was itself based on SMC code by devnull.
But we have now stripped away all but the essentials.

You should be able to read and understand this code in a few minutes,
build it, run it, and know that it's not doing anything fishy.


### How It Works Internally

We use the `AppleSMC` IOKit interface to interact from userland directly
with the System Management Controller (Mac embedded controllers),
where we adjust the Battery Charge Level Max (BCLM) parameter.

The value is permanently stored in SMC, and the tool need only be run
once (or whenever the SMC value is reset or needs to be updated).


### Download Executable

You can download a compiled executable from:
https://github.com/axonchisel/macbook-charge-limiter/releases


### Building from Source

Or you can build it yourself from source.

Just run `make` in the directory. 
An executable `macbook-charge-limiter` will be created.


### Help / Usage

```
$ ./macbook-charge-limiter -h
MacBook Charge Limiter Tool 1.0.3 -- Copyright (C) 2021 AxonChisel.net

Usage:
./macbook-charge-limiter [options] [new-limit]
    -h         : help
    -v         : verbose mode
Invoke with no arguments to read current value,
or specify value 1-100 to set new charge limit.

This software comes with ABSOLUTELY NO WARRANTY. Use at your own risk.
More info: https://github.com/axonchisel/macbook-charge-limiter
```

```
$ ./macbook-charge-limiter
100
 ```

```
$ sudo ./macbook-charge-limiter 80
 ```

```
$ ./macbook-charge-limiter -v
Current battery charge limit (1-100): 80%
 ```


### Compatibility

This tool should work with all MacBooks from ~ 2011 - 2020.

New Apple Silicon (M1) MacBooks have not been tested and may not be 
compatible due to potential SMC key changes.  

If you are interested in contributing to adapt this software to newer
computers, please keep the project philosophy in mind.


### Warranty (There isn't one. Use this at your own risk.)

BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.


### Resources

#### Hardware / SMC

- Alex Ionescu - [Apple SMC The place to be definitely, for an implant](https://www.youtube.com/watch?v=nSqpinjjgmg) (RECon 2014 video)
- [github.com/theopolis/smc-fuzzer](https://github.com/theopolis/smc-fuzzer) (more complex project we forked from in 2021)

#### Batteries

- Battery University [Charging Lithium-ion](https://batteryuniversity.com/learn/article/charging_lithium_ion_batteries)
- iFixit [What to do with a Swollen Battery](https://www.ifixit.com/Wiki/What_to_do_with_a_swollen_battery)
- Engergsoft [Improving the longevity of lithium-ion batteries](https://energsoft.com/blog/f/improving-longevity-of-lithium-ion-batteries)

