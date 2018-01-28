from RestrictedPython import compile_restricted
from RestrictedPython import utility_builtins
import RestrictedPython

# python3 load
#__metaclass__ = type

# FOR enable
#_iter_unpack_sequence_ = RestrictedPython.Guards.guarded_iter_unpack_sequence()

restricted_globals = {
        '__builtins__': utility_builtins,
        '__metaclass__': type,
        '_print_': RestrictedPython.PrintCollector,
        '_getattr_': getattr,
        '_getiter_': iter,
        '_iter_unpack_sequence_': RestrictedPython.Guards.guarded_iter_unpack_sequence
        }

source_code = """
print(1)
"""

try:
    byte_code = compile_restricted(source_code, filename='<inline>', mode='exec')
    # exec(byte_code, restricted_globals, None)
    exec(byte_code, restricted_globals, None)
except SyntaxError as e:
    print("SyntaxError")
