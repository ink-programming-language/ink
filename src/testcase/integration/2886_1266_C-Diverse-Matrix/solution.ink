// Translated from solution.cpp.

var dx = [1, 0, -1, 0, -1, -1, 1, 1];

var dy = [0, 1, 0, -1, -1, 1, -1, 1];

func fast()
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
}

var mxN = (3e5 + 5);

var oo = 0x3f3f3f3f;

var mod = (1e9 + 7);

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  if (((n == m) && (n == 1)))
  {
    write(0, "\n");
    return;
  }
  if ((n == 1))
  {
    {
      var i = 1;
      while ((i < (m + 1)))
      {
        write((i + 1), cpp_char(" "));
        i += 1;
      }
    }
  } else if ((m == 1))
  {
    {
      var i = 1;
      while ((i < (n + 1)))
      {
        write((i + 1), "\n");
        i += 1;
      }
    }
  } else
  {
    {
      var i = 1;
      while ((i < (n + 1)))
      {
        {
          var j = 1;
          while ((j < (m + 1)))
          {
            write((i * ((j + n))), cpp_char(" "));
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
  }
}

func main()
{
  fast();
  solve();
}
