// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    var arr = cpp_array(m, n);
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < m))
          {
            read(arr[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var row = 0;
    var col = 0;
    {
      var i = 0;
      while ((i < n))
      {
        var flag = true;
        {
          var j = 0;
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
      var j = 0;
      while ((j < m))
      {
        var flag = true;
        {
          var i = 0;
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
    var ans = min(row, col);
    if (((ans % 2) == 0))
    {
      write("Vivek", "\n");
    } else
    {
      write("Ashish", "\n");
    }
  }
}
