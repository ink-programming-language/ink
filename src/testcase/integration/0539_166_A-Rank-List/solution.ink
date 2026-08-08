// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var A = cpp_array(50);
  var i: dynamic;
  var con = 0;
  var a: dynamic;
  var b: dynamic;
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
