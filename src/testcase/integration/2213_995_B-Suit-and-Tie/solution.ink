// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var a = cpp_array((2 * n));
  {
    var i = 0;
    while ((i < (2 * n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ct = 0;
  var j: dynamic;
  {
    var i = 0;
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
        var k = (j - 1);
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
