// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func read(x: dynamic) -> dynamic
{
  var ch: dynamic = getchar();
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

var N: dynamic = (2e5 + 5);

var M: dynamic = 2000;

var NN: dynamic = (N * 5);

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var mx: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var res: dynamic = cpp_array(M, M);

var sr: dynamic = cpp_uninitialized();

var sb: dynamic = cpp_uninitialized();

var sy: dynamic = cpp_uninitialized();

var id: dynamic = cpp_array(N);

var r: dynamic = cpp_array(NN);

var b: dynamic = cpp_array(NN);

var y: dynamic = cpp_array(NN);

var xx: dynamic = cpp_expression("#incl");

var yy: dynamic = cpp_expression("#inclu");

func get() -> dynamic
{
  var l: dynamic = 0;
  var r: dynamic = 1000;
  var mid: dynamic = cpp_uninitialized();
  var res: dynamic = 0;
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

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  read(T);
  while (cpp_update(T, "--"))
  {
    read(s);
    read(n);
    mx = 0;
    {
      var i: dynamic = 1;
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
    var len: dynamic = get();
    sr = cpp_assign(sb, "=", cpp_assign(sy, "=", 0));
    {
      var i: dynamic = 2;
      while ((i <= len))
      {
        {
          var j: dynamic = 1;
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
      var i: dynamic = 1;
      while ((i <= len))
      {
        {
          var j: dynamic = 1;
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
      var i: dynamic = 1;
      while ((i <= len))
      {
        {
          var j: dynamic = 2;
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
      var i: dynamic = 1;
      while ((i <= len))
      {
        {
          var j: dynamic = 1;
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
      var i: dynamic = 1;
      while ((i <= n))
      {
        {
          var j: dynamic = 1;
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
      var i: dynamic = 1;
      while ((i <= len))
      {
        {
          var j: dynamic = 1;
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
