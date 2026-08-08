// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var count = 0;
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      {
        var j = ((i * 2) + 1);
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
    var i = 0;
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
