// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_array(50);
  var i: dynamic = cpp_uninitialized();
  var con: dynamic = 0;
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(n, k);
  {
    i = 0;
    while ((i < n))
    {
      read(a, b);
      A[i] = ((a * 50) - b);
      i += 1;
    }
  }
  sort(A, (A + n));
  {
    i = 0;
    while ((i < n))
    {
      if ((A[i] == A[(n - k)]))
      {
        con += 1;
      }
      i += 1;
    }
  }
  write(con);
  return 0;
}
