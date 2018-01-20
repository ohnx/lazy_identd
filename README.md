# lazy_identd
lazy identd that just replies with whatever

## Usage + Configuration

To compile, run `make`. This generates a binary, `identd`.
You will need to run this binary as root. For those who care about security, please
see the security notes below.

To use lazy_identd, run `./identd`. The identd will print out the ident that is is
using. lazy_identd checks the following places to get an ident (in this order):

1. command-line argument (e.g. `./identd myident`)
2. environment variable (e.g. `IDENT=myident ./identd`)
3. fallback to `root`

## Security

I did my best. If there are any issues, please open an issue/PR to let me know.
lazy_identd does its best to prevent issues:

1. It's basically 100 LoC...
2. Threaded so DoS shouldn't cause too much resource usage compared to forking
3. Lots of checks for input values being in bounds
4. `setuid nobody`, in case all else fails
