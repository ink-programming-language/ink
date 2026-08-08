// Translated from solution.cpp.

var s = cpp_array(101010);

func ask(x: dynamic, y: dynamic)
{
  printf("? %d %d\n", x, y);
  fflush(stdout);
  scanf("%d", (&x));
  return x;
}

func ok(n: dynamic)
{
  var i: dynamic;
  printf("! ");
  {
    i = 1;
    while ((i <= n))
    {
      printf("%d", s[i]);
      i += 1;
    }
  }
  fflush(stdout);
}

func main()
{
  var n: dynamic;
  var t: dynamic;
  var now: dynamic;
  var i: dynamic;
  var shd = 0;
  var tmp: dynamic;
  var sw = 0;
  var n1: dynamic;
  var nowi: dynamic;
  scanf("%d%d", (&n), (&t));
  if ((n == 1))
  {
    write("! ", t, "\n");
    return 0;
  }
  now = t;
  if ((n & 1))
  {
    while (1)
    {
      tmp = ask(2, n);
      if (((abs((now - tmp)) % 2) == 0))
      {
        break;
      }
      sw ^= 1;
      now = tmp;
    }
    shd = (n - now);
    if ((tmp > shd))
    {
      s[1] = 1;
    } else
    {
      s[1] = 0;
    }
    s[1] ^= sw;
    nowi = (s[1] ^ sw);
    shd = (s[1] ^ sw);
    sw ^= 1;
    now = tmp;
    {
      i = 1;
      while ((i < n))
      {
        while (1)
        {
          tmp = ask(i, (i + 1));
          if (((abs((tmp - now)) % 2) != (i % 2)))
          {
            break;
          }
          now = tmp;
          if (nowi)
          {
            shd -= 1;
          } else
          {
            shd += 1;
          }
          nowi ^= 1;
          sw ^= 1;
        }
        shd = (i - shd);
        n1 = (((((i + 1) + tmp) - now)) / 2);
        s[(i + 1)] = (n1 - shd);
        shd = n1;
        s[(i + 1)] ^= sw;
        s[(i + 1)] ^= 1;
        nowi = ((s[(i + 1)] ^ sw) ^ 1);
        now = tmp;
        i += 1;
      }
    }
    ok(n);
  } else
  {
    {
      i = 1;
      while ((i <= n))
      {
        while (1)
        {
          tmp = ask(i, i);
          if (((abs((now - tmp)) % 2) == (i % 2)))
          {
            break;
          }
          now = tmp;
          sw ^= 1;
        }
        shd = ((i - 1) - shd);
        n1 = ((((i + tmp) - now)) / 2);
        s[i] = (n1 - shd);
        shd = n1;
        s[i] ^= sw;
        s[i] ^= 1;
        now = tmp;
        i += 1;
      }
    }
    ok(n);
  }
  return 0;
}
