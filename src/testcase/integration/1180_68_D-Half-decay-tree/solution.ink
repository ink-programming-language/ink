// Translated from solution.cpp.

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(x);
}

func cpp_name(x: dynamic) -> dynamic
{
  write(( (x) ? "true" : "false"));
}

func cpp_name(x: dynamic) -> dynamic
{
  cpp_name( (1) ? "(" : "");
  cpp_name(x.first);
  cpp_name( (1) ? ", " : " ");
  cpp_name(x.second);
  cpp_name( (1) ? ")" : "");
}

func cpp_name(x: dynamic) -> dynamic
{
  cpp_name( (1) ? "{" : "");
  var cpp_name: dynamic = 0;
  for (var v: dynamic in x)
  {
    cpp_name( (cpp_name) ?  (1) ? ", " : " " : "");
    cpp_name(v);
    cpp_name = 1;
  }
  cpp_name( (1) ? "}" : "");
}

func cpp_name(x: dynamic) -> dynamic
{
  cpp_name( (1) ? "{" : "");
  var cpp_name: dynamic = 0;
  for (var v: dynamic in x)
  {
    cpp_name( (cpp_name) ?  (1) ? ", " : " " : "");
    cpp_name(v);
    cpp_name = 1;
  }
  cpp_name( (1) ? "}" : "");
}

func cpp_name(x: dynamic) -> dynamic
{
  cpp_name( (1) ? "{" : "");
  var cpp_name: dynamic = 0;
  for (var v: dynamic in x)
  {
    cpp_name( (cpp_name) ?  (1) ? ", " : " " : "");
    cpp_name(v);
    cpp_name = 1;
  }
  cpp_name( (1) ? "}" : "");
}

func cpp_name(x: dynamic) -> dynamic
{
  cpp_name( (1) ? "{" : "");
  var cpp_name: dynamic = 0;
  for (var v: dynamic in x)
  {
    cpp_name( (cpp_name) ?  (1) ? ", " : " " : "");
    cpp_name(v);
    cpp_name = 1;
  }
  cpp_name( (1) ? "}" : "");
}

func pr() -> dynamic
{
  write("\n");
}

func pr(a: dynamic, b: dynamic...) -> dynamic
{
  cpp_name(a);
  if (cpp_sizeof(b))
  {
    cpp_name(cpp_char(" "));
  }
  pr(cpp_expand(b));
}

var MN: dynamic = 33;

var h: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var sm: dynamic = cpp_uninitialized();

var val: dynamic = cpp_uninitialized();

func main() -> dynamic
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
        var mx: dynamic = 0;
        var prob: dynamic = 1;
        var ans: dynamic = 0;
        {
          i = 0;
          while ((i < h))
          {
            var lsum: dynamic = (val[x] + sm[(x << 1)]);
            var rsum: dynamic = (val[x] + sm[(((x << 1)) | 1)]);
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
