// Importing a module is allowed, but its private member remains inaccessible.
import provider as lib;

func main(): i32
{
  return lib.privateFn();
}
