// Translated from solution.cpp.

var LL = dynamic;

var uLL = dynamic;

var N = (2e5 + 10);

var inf = 1e9;

func rd()
{
  var x = 0;
  var w = 1;
  var ch = 0;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      w = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = ((x * 10) + ((ch ^ 48)));
    ch = getchar();
  }
  return (x * w);
}

var to = cpp_array((N << 1));

var nt = cpp_array((N << 1));

var hd = cpp_array(N);

var tot = 1;

func adde(x: dynamic, y: dynamic)
{
  tot += 1;
  to[tot] = y;
  nt[tot] = hd[x];
  hd[x] = tot;
  tot += 1;
  to[tot] = x;
  nt[tot] = hd[y];
  hd[y] = tot;
}

var co = [cpp_char("W"), cpp_char("B")];

var n: dynamic;

var m: dynamic;

var a = cpp_array(N);

var sq = cpp_array(N);

var a1 = cpp_array(N);

var a2 = cpp_array(N);

func cmp(aa: dynamic, bb: dynamic)
{
  return (a[aa] < a[bb]);
}

func main()
{
  n = rd();
  m = rd();
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = rd();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      adde(rd(), rd());
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= max(n, m)))
    {
      a1[i] = -1;
      a2[i] = (inf + 1);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      sq[i] = i;
      i += 1;
    }
  }
  sort((sq + 1), ((sq + n) + 1), cmp);
  {
    var i = 1;
    while ((i <= n))
    {
      var x = sq[i];
      {
        var j = hd[x];
        while (j)
        {
          var y = to[j];
          if (((a[y] > a[x]) || ((((~a1[x])) && ((~a1[y]))))))
          {
            j = nt[j];
            continue;
          }
          if ((a[x] == a[y]))
          {
            a2[(j >> 1)] = a[x];
            if ((a1[x] < 0))
            {
              if ((a1[y] < 0))
              {
                a1[y] = 0;
              }
              a1[x] = (a1[y] ^ 1);
            } else
            {
              a1[y] = (a1[x] ^ 1);
            }
          } else if ((~a1[y]))
          {
            a2[(j >> 1)] = (a[x] - a[y]);
            a1[x] = a1[y];
          }
          j = nt[j];
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a1[i] < 0))
      {
        puts("-1");
        return 0;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      putchar(co[a1[i]]);
      i += 1;
    }
  }
  puts("");
  {
    var i = 1;
    while ((i <= m))
    {
      printf("%d\n", min(a2[i], inf));
      i += 1;
    }
  }
  return 0;
}
