// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(100000);

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(A[i]);
      i += 1;
    }
  }
  var x: dynamic = A[(n - 1)];
  var j: dynamic =  ((A[0] < x)) ? 1 : 0;
  {
    var i: dynamic = 1;
    while ((i < (n - 1)))
    {
      if ((A[i] <= x))
      {
        var aj: dynamic = A[j];
        A[j] = A[i];
        A[i] = aj;
        j += 1;
      }
      i += 1;
    }
  }
  A[(n - 1)] = A[j];
  A[j] = x;
  {
    var i: dynamic = 0;
    while ((i < j))
    {
      if (i)
      {
        write(cpp_char(" "));
      }
      write(A[i]);
      i += 1;
    }
  }
  if (j)
  {
    write(cpp_char(" "));
  }
  write(cpp_char("["), A[j], cpp_char("]"));
  {
    var i: dynamic = (j + 1);
    while ((i < n))
    {
      write(cpp_char(" "), A[i]);
      i += 1;
    }
  }
  write("\n");
}
