// Translated from solution.cpp.

var MOD = (1E9 + 7);

var N = (3000000 + 5);

var dx = [-1, 1, 0, 0, -1, -1, 1, 1];

var dy = [0, 0, -1, 1, -1, 1, -1, 1];

var tot: dynamic;

var f = cpp_array(N);

var X = cpp_array(N);

var Y = cpp_array(N);

var a = cpp_array(N);

var len: dynamic;

var seq = cpp_array(N);

var n: dynamic;

var m: dynamic;

var father = cpp_array(N);

var str = cpp_array(N);

var c: dynamic;

func get(x: dynamic)
{
  var l = 1;
  seq[1] = x;
  {
    while (f[seq[l]])
    {
      seq[(l + 1)] = f[seq[l]];
      l += 1;
    }
  }
  {
    var i = 1;
    while ((i < l))
    {
      f[seq[i]] = seq[l];
      i += 1;
    }
  }
  return seq[l];
}

func getPos(x: dynamic)
{
  if ((!tot))
  {
    return x;
  }
  var i = 1;
  var last: dynamic;
  if ((x < X[a[i]]))
  {
    return x;
  }
  x -= (X[a[i]] - 1);
  {
    last = cpp_update(i, "++");
    while ((i <= tot))
    {
      if ((X[a[i]] > Y[a[last]]))
      {
        if ((x > ((X[a[i]] - Y[a[last]]) - 1)))
        {
          x -= ((X[a[i]] - Y[a[last]]) - 1);
          last = i;
        } else
        {
          return (Y[a[last]] + x);
        }
      }
      i += 1;
    }
  }
  return (Y[a[last]] + x);
}

class Query
{
  var right: dynamic;
  var left: dynamic;
}

var query = cpp_array(N);

func main()
{
  scanf("%s", (str + 1));
  scanf("%d%d", (&m), (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d%d", (&query[i].left), (&query[i].right));
      i += 1;
    }
  }
  memset(f, 0, cpp_sizeof((f)));
  memset(father, 0, cpp_sizeof((father)));
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      if ((query[i].right < m))
      {
        var pos = getPos((query[i].right + 1));
        if ((pos <= m))
        {
          var k = (query[i].left + 1);
          if ((k > query[i].right))
          {
            k = query[i].left;
          }
          var newPos = getPos(k);
          var lim = ((((query[i].right << 1)) - query[i].left) + 1);
          var j: dynamic;
          X[i] = pos;
          {
            j = (query[i].right + 1);
            while (((j <= lim) && (pos <= m)))
            {
              father[pos] = newPos;
              f[pos] = (pos + 1);
              k += 2;
              if ((k <= query[i].right))
              {
                newPos = get((get((newPos + 1)) + 1));
              } else
              {
                k = query[i].left;
                newPos = getPos(k);
              }
              Y[i] = pos;
              j += 1;
              pos = get(pos);
            }
          }
          {
            j = tot;
            while ((j && (X[a[j]] > X[i])))
            {
              a[(j + 1)] = a[j];
              j -= 1;
            }
          }
          a[(j + 1)] = i;
          tot += 1;
        }
      }
      i -= 1;
    }
  }
  var cnt = 0;
  {
    var i = 1;
    while ((i <= m))
    {
      father[i] = if (((!father[i]))) cpp_update(cnt, "++") else father[father[i]];
      printf("%c", str[father[i]]);
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
