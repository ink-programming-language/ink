// Translated from solution.cpp.

func main() -> dynamic
{
  var K: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  read(K, A, B);
  if ((K <= A))
  {
    write(1, "\n");
  } else if (((A - B) <= 0))
  {
    write(-1, "\n");
  } else
  {
    var t: dynamic = (K - A);
    write(((((((t + ((A - B))) - 1)) / ((A - B))) * 2) + 1), "\n");
  }
}
