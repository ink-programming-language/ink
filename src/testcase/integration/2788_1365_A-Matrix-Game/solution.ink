// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    var arr: dynamic = cpp_array(m, n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        {
          var j: dynamic = 0;
          while ((j < m))
          {
            read(arr[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var row: dynamic = 0;
    var col: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var flag: dynamic = true;
        {
          var j: dynamic = 0;
          while ((j < m))
          {
            if ((arr[i][j] == 1))
            {
              flag = false;
            }
            j += 1;
          }
        }
        if (flag)
        {
          row += 1;
        }
        i += 1;
      }
    }
    {
      var j: dynamic = 0;
      while ((j < m))
      {
        var flag: dynamic = true;
        {
          var i: dynamic = 0;
          while ((i < n))
          {
            if ((arr[i][j] == 1))
            {
              flag = false;
            }
            i += 1;
          }
        }
        if (flag)
        {
          col += 1;
        }
        j += 1;
      }
    }
    var ans: dynamic = min(row, col);
    if (((ans % 2) == 0))
    {
      write("Vivek", "\n");
    } else
    {
      write("Ashish", "\n");
    }
  }
}
