// Translated from solution.cpp.

var n: dynamic;

var d: dynamic;

var x: dynamic;

var a = cpp_array(100002);

var b = cpp_array(100002);

var c = cpp_array(100002);

var q = cpp_array(100002);

var pos = cpp_array(100002);

var sb = 0;

func getNextX()
{
  x = ((((x * 37) + 10007)) % 1000000007);
  return x;
}

func initAB()
{
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = (i + 1);
      i = (i + 1);
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      swap(a[i], a[(getNextX() % ((i + 1)))]);
      i = (i + 1);
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((i < d))
      {
        b[i] = 1;
      } else
      {
        b[i] = 0;
      }
      i = (i + 1);
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      swap(b[i], b[(getNextX() % ((i + 1)))]);
      i = (i + 1);
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  read(n, d, x);
  initAB();
  {
    var i = 0;
    while ((i < n))
    {
      if (b[i])
      {
        sb += 1;
        q[sb] = i;
      }
      pos[a[i]] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = n;
        while ((j > (n - 30)))
        {
          if ((j < 1))
          {
            break;
          }
          if (((i >= pos[j]) && b[(i - pos[j])]))
          {
            c[i] = j;
            break;
          }
          j -= 1;
        }
      }
      if ((c[i] != 0))
      {
        i += 1;
        continue;
      }
      var v = 0;
      {
        var j = 1;
        while ((j <= sb))
        {
          if ((q[j] > i))
          {
            break;
          }
          v = max(v, a[(i - q[j])]);
          j += 1;
        }
      }
      c[i] = v;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      write(c[i], "\n");
      i += 1;
    }
  }
  return 0;
}
