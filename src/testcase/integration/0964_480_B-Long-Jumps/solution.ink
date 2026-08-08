// Translated from solution.cpp.

var a = cpp_array(100005);

var b = cpp_array(100005);

var ansx: dynamic;

var ansy: dynamic;

var n: dynamic;

var qst: dynamic;

var qed: dynamic;

var now: dynamic;

var ansk: dynamic;

var x: dynamic;

var y: dynamic;

var l: dynamic;

func find(nx: dynamic, t: dynamic)
{
  qst = 1;
  qed = 1;
  now = b[1];
  while ((qst <= n))
  {
    if ((now == nx))
    {
      if ((t == 0))
      {
        return 1;
      } else
      {
        if ((((a[qed] - x) >= 0) || ((a[qed] + y) <= l)))
        {
          return 1;
        }
      }
    }
    if (((qed < qst) && (now > nx)))
    {
      now -= b[qed];
      qed += 1;
    } else
    {
      qst += 1;
      now += b[qst];
    }
  }
  return 0;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var m: dynamic;
  scanf("%d%d%d%d", (&n), (&l), (&x), (&y));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      b[(i - 1)] = (a[i] - a[(i - 1)]);
      i += 1;
    }
  }
  n -= 1;
  if ((!find(x, 0)))
  {
    ansx += 1;
  }
  if ((!find(y, 0)))
  {
    ansy += 1;
  }
  if (((x == y) && ansx))
  {
    printf("%d\n%d\n", 1, x);
  } else if ((((x != y) && ansx) && ansy))
  {
    if (find((x + y), 0))
    {
      ansk += 1;
    }
    if (ansk)
    {
      printf("1\n%d\n", (x + a[qed]));
    } else
    {
      if (find((y - x), 1))
      {
        ansk += 1;
      }
      if (ansk)
      {
        printf("1\n");
        if (((a[qed] - x) >= 0))
        {
          printf("%d\n", (a[qed] - x));
        } else
        {
          printf("%d\n", (a[qed] + y));
        }
      } else
      {
        printf("2\n%d %d\n", x, y);
      }
    }
  } else
  {
    if (((ansx + ansy) == 2))
    {
      printf("2\n%d %d\n", x, y);
    } else
    {
      printf("%d\n", (ansx + ansy));
      if (ansx)
      {
        printf("%d\n", x);
      }
      if (ansy)
      {
        printf("%d\n", y);
      }
    }
  }
  return 0;
}
