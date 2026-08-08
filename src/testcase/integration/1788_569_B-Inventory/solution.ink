// Translated from solution.cpp.

var a = cpp_array(100000);

var c = cpp_array(100001);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  read(n);
  var unused: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      unused.insert((i + 1));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      if ((a[i] <= n))
      {
        c[a[i]] += 1;
        if ((c[a[i]] == 1))
        {
          unused.erase(unused.find(a[i]));
        } else
        {
          a[i] = 0;
        }
      } else
      {
        a[i] = 0;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((a[i] == 0))
      {
        a[i] = (*unused.begin());
        unused.erase(unused.begin());
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
