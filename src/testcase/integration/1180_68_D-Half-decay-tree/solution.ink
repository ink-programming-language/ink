// Translated from solution.cpp.

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write(x);
}

func cpp_name(x: dynamic)
{
  write((if (x) "true" else "false"));
}

func cpp_name(x: dynamic)
{
  cpp_name(if (1) "(" else "");
  cpp_name(x.first);
  cpp_name(if (1) ", " else " ");
  cpp_name(x.second);
  cpp_name(if (1) ")" else "");
}

func cpp_name(x: dynamic)
{
  cpp_name(if (1) "{" else "");
  var cpp_name = 0;
  for (var v in x)
  {
    cpp_name(if (cpp_name) if (1) ", " else " " else "");
    cpp_name(v);
    cpp_name = 1;
  }
  cpp_name(if (1) "}" else "");
}

func cpp_name(x: dynamic)
{
  cpp_name(if (1) "{" else "");
  var cpp_name = 0;
  for (var v in x)
  {
    cpp_name(if (cpp_name) if (1) ", " else " " else "");
    cpp_name(v);
    cpp_name = 1;
  }
  cpp_name(if (1) "}" else "");
}

func cpp_name(x: dynamic)
{
  cpp_name(if (1) "{" else "");
  var cpp_name = 0;
  for (var v in x)
  {
    cpp_name(if (cpp_name) if (1) ", " else " " else "");
    cpp_name(v);
    cpp_name = 1;
  }
  cpp_name(if (1) "}" else "");
}

func cpp_name(x: dynamic)
{
  cpp_name(if (1) "{" else "");
  var cpp_name = 0;
  for (var v in x)
  {
    cpp_name(if (cpp_name) if (1) ", " else " " else "");
    cpp_name(v);
    cpp_name = 1;
  }
  cpp_name(if (1) "}" else "");
}

func pr()
{
  write("\n");
}

func pr(a: dynamic, b: dynamic...)
{
  cpp_name(a);
  if (cpp_sizeof(b))
  {
    cpp_name(cpp_char(" "));
  }
  pr(cpp_expand(b));
}

var MN = 33;

var h: dynamic;

var q: dynamic;

var i: dynamic;

var x: dynamic;

var y: dynamic;

var s: dynamic;

var sm: dynamic;

var val: dynamic;

func main()
{
  scanf("%d%d", (&h), (&q));
  {
    while (q)
    {
      read(s);
      if ((s == "add"))
      {
        scanf("%d%d", (&x), (&y));
        val[x] += y;
        while (x)
        {
          sm[x] += y;
          x >>= 1;
        }
      } else
      {
        x = 1;
        var mx = 0;
        var prob = 1;
        var ans = 0;
        {
          i = 0;
          while ((i < h))
          {
            var lsum = (val[x] + sm[(x << 1)]);
            var rsum = (val[x] + sm[(((x << 1)) | 1)]);
            if ((lsum > rsum))
            {
              ans += ((prob / 2.0) * max(mx, lsum));
              mx = max(mx, rsum);
              x = ((x << 1));
            } else
            {
              ans += ((prob / 2.0) * max(mx, rsum));
              mx = max(mx, lsum);
              x = (((x << 1)) | 1);
            }
            prob /= 2;
            i += 1;
          }
        }
        ans += (prob * max(mx, val[x]));
        printf("%.6lf\n", ans);
      }
      q -= 1;
    }
  }
  return 0;
}
