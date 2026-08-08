// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  read(n);
  var maxx = 0;
  var c = 0;
  var a = cpp_array(n);
  var count = 1;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((a[i] == a[(i + 1)]))
      {
        count += 1;
      } else if ((a[i] != a[(i + 1)]))
      {
        if ((count > 0))
        {
          if ((maxx == 0))
          {
            maxx = a[i];
            c = count;
          } else if ((maxx < a[i]))
          {
            maxx = a[i];
            c = count;
          } else if ((maxx == a[i]))
          {
            if ((c < count))
            {
              c = count;
            }
          }
        }
        count = 1;
      }
      i += 1;
    }
  }
  write(c, cpp_char("\n"));
  return 0;
}
