// Translated from solution.cpp.

var int_cpp = dynamic;

func read(x: dynamic)
{
  var ch = getchar();
  x = 0;
  while ((!isdigit(ch)))
  {
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = (((x * 10) + ch) - 48);
    ch = getchar();
  }
}

var N = (2e5 + 5);

var M = 2000;

var NN = (N * 5);

var n: dynamic;

var s: dynamic;

var mx: dynamic;

var a = cpp_array(N);

var res = cpp_array(M, M);

var sr: dynamic;

var sb: dynamic;

var sy: dynamic;

var id = cpp_array(N);

var r = cpp_array(NN);

var b = cpp_array(NN);

var y = cpp_array(NN);

var xx = cpp_expression("#incl");

var yy = cpp_expression("#inclu");

func get()
{
  var l = 0;
  var r = 1000;
  var mid: dynamic;
  var res = 0;
  while ((l <= r))
  {
    mid = ((l + r) >> 1);
    if (((s <= ((mid * mid) - (((mid / 2)) * ((mid / 2))))) && (a[1] <= (mid * ((((mid + 1)) / 2))))))
    {
      res = mid;
      r = (mid - 1);
    } else
    {
      l = (mid + 1);
    }
  }
  return res;
}

func main()
{
  var T: dynamic;
  read(T);
  while (cpp_update(T, "--"))
  {
    read(s);
    read(n);
    mx = 0;
    {
      var i = 1;
      while ((i <= n))
      {
        read(a[i]);
        id[i] = i;
        if ((a[i] > a[mx]))
        {
          mx = i;
        }
        i += 1;
      }
    }
    id[1] = mx;
    id[mx] = 1;
    swap(a[1], a[mx]);
    var len = get();
    sr = cpp_assign(sb, "=", cpp_assign(sy, "=", 0));
    {
      var i = 2;
      while ((i <= len))
      {
        {
          var j = 1;
          while ((j <= len))
          {
            r[cpp_update(sr, "++")] = [i, j];
            j += 2;
          }
        }
        i += 2;
      }
    }
    {
      var i = 1;
      while ((i <= len))
      {
        {
          var j = 1;
          while ((j <= len))
          {
            b[cpp_update(sb, "++")] = [i, j];
            j += 2;
          }
        }
        i += 2;
      }
    }
    {
      var i = 1;
      while ((i <= len))
      {
        {
          var j = 2;
          while ((j <= len))
          {
            y[cpp_update(sy, "++")] = [i, j];
            j += 2;
          }
        }
        i += 2;
      }
    }
    {
      var i = 1;
      while ((i <= len))
      {
        {
          var j = 1;
          while ((j <= len))
          {
            res[i][j] = 0;
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 1;
          while ((j <= a[i]))
          {
            if (sr)
            {
              res[r[sr].xx][r[sr].yy] = id[i];
              sr -= 1;
            } else if (sb)
            {
              res[b[sb].xx][b[sb].yy] = id[i];
              sb -= 1;
            } else
            {
              res[y[sy].xx][y[sy].yy] = id[i];
              sy -= 1;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", len);
    {
      var i = 1;
      while ((i <= len))
      {
        {
          var j = 1;
          while ((j <= len))
          {
            printf("%d ", res[i][j]);
            j += 1;
          }
        }
        puts("");
        i += 1;
      }
    }
  }
  return 0;
}
