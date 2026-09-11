// Translated from solution.cpp.

var N: dynamic = (2e5 + 9);

var Maxn: dynamic = 2e5;

func read() -> dynamic
{
  var x: dynamic = 0;
  var F: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    F =  (((ch == cpp_char("-")))) ? -1 : 1;
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = ((((x << 1)) + ((x << 3))) + ((ch & 15)));
    ch = getchar();
  }
  return (x * F);
}

func write(x: dynamic) -> dynamic
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

func write(x: dynamic, ch: dynamic) -> dynamic
{
  write(x);
  putchar(ch);
}

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(N);

var sum: dynamic = cpp_array(N);

var f: dynamic = cpp_array(N);

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      vis[read()] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= (Maxn + 1)))
    {
      sum[i] = (sum[(i - 1)] + vis[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = Maxn;
    while ((i >= 1))
    {
      f[i] =  ((vis[i] > 1)) ? (f[(i + 1)] + 1) : 0;
      i -= 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= Maxn))
    {
      ans = max(ans, (sum[(i + f[i])] - sum[(i - 2)]));
      ans = max(ans, (vis[i] + vis[(i + 1)]));
      i += 1;
    }
  }
  write(ans, "\n");
  {
    var i: dynamic = 2;
    while ((i <= Maxn))
    {
      if (((sum[(i + f[i])] - sum[(i - 2)]) == ans))
      {
        {
          var cnt: dynamic = 1;
          while ((cnt <= vis[(i - 1)]))
          {
            write((i - 1), cpp_char(" "));
            cnt += 1;
          }
        }
        {
          var j: dynamic = i;
          while ((j <= ((i + f[i]) - 1)))
          {
            {
              var cnt: dynamic = 1;
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
          var cnt: dynamic = 1;
          while ((cnt <= vis[(i + f[i])]))
          {
            write((i + f[i]), cpp_char(" "));
            cnt += 1;
          }
        }
        {
          var j: dynamic = ((i + f[i]) - 1);
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
          var j: dynamic = 1;
          while ((j <= vis[i]))
          {
            write(i, cpp_char(" "));
            j += 1;
          }
        }
        {
          var j: dynamic = 1;
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
