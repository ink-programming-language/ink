// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array((2 * n));
  {
    var i: dynamic = 0;
    while ((i < (2 * n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ct: dynamic = 0;
  var j: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (2 * n)))
    {
      {
        j = (i + 1);
        while ((j < (2 * n)))
        {
          if ((a[j] == a[i]))
          {
            break;
          }
          j += 1;
        }
      }
      {
        var k: dynamic = (j - 1);
        while ((k > i))
        {
          a[(k + 1)] = a[k];
          ct += 1;
          k -= 1;
        }
      }
      i += 1;
      i += 1;
    }
  }
  write(ct, "\n");
}
