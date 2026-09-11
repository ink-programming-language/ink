// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var count: dynamic = 0;
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      {
        var j: dynamic = ((i * 2) + 1);
        while ((j < n))
        {
          a[i] ^= a[j];
          j += (i + 1);
        }
      }
      count += a[i];
      i -= 1;
    }
  }
  write(count, "\n");
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (a[i])
      {
        write((i + 1), " ");
      }
      i += 1;
    }
  }
}
