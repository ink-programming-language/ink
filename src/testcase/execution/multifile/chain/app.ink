// The entry module depends only on the middle module's exported function.
from middle import answer as remote;

// The saved executable must traverse both imported edges and return 42.
func main(): i32
{
  return remote() + 1;
}
