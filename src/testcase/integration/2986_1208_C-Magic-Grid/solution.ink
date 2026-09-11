// Translated from solution.cpp.

var N: dynamic = (1e5 + 9);

var mod: dynamic = (1e9 + 7);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array(n, n);
  var c: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n / 2)))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
