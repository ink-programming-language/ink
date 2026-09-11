// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var maxx: dynamic = 0;
  var c: dynamic = 0;
  var a: dynamic = cpp_array(n);
  var count: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
