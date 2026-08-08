// Translated from solution.cpp.

var N = (1e5 + 9);

var mod = (1e9 + 7);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  read(n);
  var a = cpp_array(n, n);
  var c = 0;
  {
    var i = 0;
    while ((i < (n / 2)))
    {
      {
        var j = 0;
        while ((j < (n / 2)))
        {
          a[i][j] = (4 * c);
          a[(i + (n / 2))][j] = ((4 * c) + 1);
          a[i][(j + (n / 2))] = ((4 * c) + 2);
          a[(i + (n / 2))][(j + (n / 2))] = ((4 * c) + 3);
          c += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          write(a[i][j], " ");
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
}
