// Translated from solution.cpp.

var N = (2e5 + 9);

var Maxn = 2e5;

func read()
{
  var x = 0;
  var F = 1;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    F = if (((ch == cpp_char("-")))) -1 else 1;
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = ((((x << 1)) + ((x << 3))) + ((ch & 15)));
    ch = getchar();
  }
  return (x * F);
}

func write(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = ((~x) + 1);
  }
  if ((x > 9))
  {
    write((x / 10));
  }
  putchar((((x % 10)) | 48));
}

func write(x: dynamic, ch: dynamic)
{
  write(x);
  putchar(ch);
}

var n: dynamic;

var ans: dynamic;

var vis = cpp_array(N);

var sum = cpp_array(N);

var f = cpp_array(N);

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      vis[read()] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (Maxn + 1)))
    {
      sum[i] = (sum[(i - 1)] + vis[i]);
      i += 1;
    }
  }
  {
    var i = Maxn;
    while ((i >= 1))
    {
      f[i] = if ((vis[i] > 1)) (f[(i + 1)] + 1) else 0;
      i -= 1;
    }
  }
  {
    var i = 2;
    while ((i <= Maxn))
    {
      ans = max(ans, (sum[(i + f[i])] - sum[(i - 2)]));
      ans = max(ans, (vis[i] + vis[(i + 1)]));
      i += 1;
    }
  }
  write(ans, "\n");
  {
    var i = 2;
    while ((i <= Maxn))
    {
      if (((sum[(i + f[i])] - sum[(i - 2)]) == ans))
      {
        {
          var cnt = 1;
          while ((cnt <= vis[(i - 1)]))
          {
            write((i - 1), cpp_char(" "));
            cnt += 1;
          }
        }
        {
          var j = i;
          while ((j <= ((i + f[i]) - 1)))
          {
            {
              var cnt = 1;
              while ((cnt < vis[j]))
              {
                write(j, cpp_char(" "));
                cnt += 1;
              }
            }
            j += 1;
          }
        }
        {
          var cnt = 1;
          while ((cnt <= vis[(i + f[i])]))
          {
            write((i + f[i]), cpp_char(" "));
            cnt += 1;
          }
        }
        {
          var j = ((i + f[i]) - 1);
          while ((j >= i))
          {
            write(j, cpp_char(" "));
            j -= 1;
          }
        }
        break;
      }
      if (((vis[i] + vis[(i + 1)]) == ans))
      {
        {
          var j = 1;
          while ((j <= vis[i]))
          {
            write(i, cpp_char(" "));
            j += 1;
          }
        }
        {
          var j = 1;
          while ((j <= vis[(i + 1)]))
          {
            write((i + 1), cpp_char(" "));
            j += 1;
          }
        }
        break;
      }
      i += 1;
    }
  }
  return 0;
}
