// Translated from solution.cpp.

var MOD: dynamic = (1E9 + 7);

var N: dynamic = (3000000 + 5);

var dx: dynamic = [-1, 1, 0, 0, -1, -1, 1, 1];

var dy: dynamic = [0, 0, -1, 1, -1, 1, -1, 1];

var tot: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

var X: dynamic = cpp_array(N);

var Y: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var len: dynamic = cpp_uninitialized();

var seq: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var father: dynamic = cpp_array(N);

var str: dynamic = cpp_array(N);

var c: dynamic = cpp_uninitialized();

func get(x: dynamic) -> dynamic
{
  var l: dynamic = 1;
  seq[1] = x;
  {
    while (f[seq[l]])
    {
      seq[(l + 1)] = f[seq[l]];
      l += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < l))
    {
      f[seq[i]] = seq[l];
      i += 1;
    }
  }
  return seq[l];
}

func getPos(x: dynamic) -> dynamic
{
  if ((!tot))
  {
    return x;
  }
  var i: dynamic = 1;
  var last: dynamic = cpp_uninitialized();
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
  var right: dynamic = cpp_uninitialized();
  var left: dynamic = cpp_uninitialized();
}

var query: dynamic = cpp_array(N);

func main() -> dynamic
{
  scanf("%s", (str + 1));
  scanf("%d%d", (&m), (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d%d", (&query[i].left), (&query[i].right));
      i += 1;
    }
  }
  memset(f, 0, cpp_sizeof((f)));
  memset(father, 0, cpp_sizeof((father)));
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      if ((query[i].right < m))
      {
        var pos: dynamic = getPos((query[i].right + 1));
        if ((pos <= m))
        {
          var k: dynamic = (query[i].left + 1);
          if ((k > query[i].right))
          {
            k = query[i].left;
          }
          var newPos: dynamic = getPos(k);
          var lim: dynamic = ((((query[i].right << 1)) - query[i].left) + 1);
          var j: dynamic = cpp_uninitialized();
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
  var cnt: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      father[i] =  (((!father[i]))) ? cpp_update(cnt, "++") : father[father[i]];
      printf("%c", str[father[i]]);
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
