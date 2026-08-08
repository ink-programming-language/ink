// Translated from solution.cpp.

var n: dynamic;

var A = cpp_array(100000);

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(A[i]);
      i += 1;
    }
  }
  var x = A[(n - 1)];
  var j = if ((A[0] < x)) 1 else 0;
  {
    var i = 1;
    while ((i < (n - 1)))
    {
      if ((A[i] <= x))
      {
        var aj = A[j];
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
    var i = 0;
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
    var i = (j + 1);
    while ((i < n))
    {
      write(cpp_char(" "), A[i]);
      i += 1;
    }
  }
  write("\n");
}
