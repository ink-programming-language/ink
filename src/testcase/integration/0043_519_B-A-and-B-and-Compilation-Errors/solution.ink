// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var a = cpp_array(3);
  var q: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < 3))
    {
      a[i] = 0;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 3))
    {
      {
        var j = 0;
        while ((j < (n - i)))
        {
          read(q);
          a[i] += q;
          j += 1;
        }
      }
      i += 1;
    }
  }
  write((a[0] - a[1]), "\n");
  write((a[1] - a[2]));
  return 0;
}
