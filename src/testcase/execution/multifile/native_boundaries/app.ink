from library import abs as magnitude;
from library import localValue;
import library as api;

// Both Ink import forms preserve the provider's ABI and binding while selecting the right execution path.
func main(): i32
{
  return localValue(10) + api.inkExportedValue(10) + api.privateWrapped() + magnitude(-16);
}
