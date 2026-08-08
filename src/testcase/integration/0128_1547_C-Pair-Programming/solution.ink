// Translated from solution.cpp.

var N = (1e5 + 10);

var t: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var tot: dynamic;

var a = cpp_array(N);

var b = cpp_array(N);

var ans = cpp_array(N);

func read(x: dynamic)
{
  x = 0;
  var c = getchar();
  var op = 1;
  {
    while (((c < cpp_char("0")) || (c > cpp_char("9"))))
    {
      if ((c == cpp_char("-")))
      {
        op = -1;
      }
      c = getchar();
    }
  }
  {
    while (((c <= cpp_char("9")) && (c >= cpp_char("0"))))
    {
      x = (((((x << 3)) + ((x << 1))) + c) - cpp_char("0"));
      c = getchar();
    }
  }
  x *= op;
}

func main()
{
  read(t);
  while (cpp_update(t, "--"))
  {
    tot = 0;
    read(k);
    read(n);
    read(m);
    {
      var i = 1;
      while ((i <= n))
      {
        read(a[i]);
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= m))
      {
        read(b[i]);
        i += 1;
      }
    }
    var i = 1;
    var j = 1;
    while (((i <= n) || (j <= m)))
    {
      while (((a[i] == 0) && (i <= n)))
      {
        k += 1;
        ans[cpp_update(tot, "++")] = 0;
        i += 1;
      }
      while (((b[j] == 0) && (j <= m)))
      {
        k += 1;
        ans[cpp_update(tot, "++")] = 0;
        j += 1;
      }
      if (((a[i] <= k) && (i <= n)))
      {
        ans[cpp_update(tot, "++")] = a[i];
        i += 1;
      } else if (((b[j] <= k) && (j <= m)))
      {
        ans[cpp_update(tot, "++")] = b[j];
        j += 1;
      } else
      {
        break;
      }
    }
    if (((i <= n) || (j <= m)))
    {
      printf("-1");
    } else
    {
      {
        var i = 1;
        while ((i <= tot))
        {
          printf("%d ", ans[i]);
          i += 1;
        }
      }
    }
    putchar(cpp_char("\n"));
  }
  return 0;
}
