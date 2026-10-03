// A from import must reject a private definition before a local alias is published.
from provider import hiddenValue as hidden;

func main(): i32
{
  return hidden();
}
