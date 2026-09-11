// Translated from solution.cpp.

var pi: dynamic = acos(-1.0);

var ax: dynamic = cpp_uninitialized();

var ay: dynamic = cpp_uninitialized();

var bx: dynamic = cpp_uninitialized();

var by: dynamic = cpp_uninitialized();

func dis(x: dynamic, y: dynamic) -> dynamic
{
  return sqrt(((x * x) + (y * y)));
}

func get(a: dynamic, w: dynamic) -> dynamic
{
  var v: dynamic = 0;
  var num: dynamic = 100;
  {
    var i: dynamic = 0;
    while ((i <= 100))
    {
      var x: dynamic = (ax + ((((bx - ax)) * i) / cpp_cast(num)));
      var y: dynamic = (ay + ((((by - ay)) * i) / cpp_cast(num)));
      var p: dynamic = (atan2(y, x) - a);
      if ((p > (2 * pi)))
      {
        p -= (2 * pi);
      }
      while ((p < 0))
      {
        p += (2 * pi);
      }
      if ((p > ((2 * pi) - p)))
      {
        p = ((2 * pi) - p);
      }
      var t: dynamic = ( (((w < 1e-5))) ? ( ((i == 0)) ? 0 : 1e10) : (p / w));
      v = max(v, (hypot((x - ax), (y - ay)) / t));
      i += 1;
    }
  }
  return v;
}

var x: dynamic = cpp_array(10010);

var y: dynamic = cpp_array(10010);

var a: dynamic = cpp_array(10010);

var w: dynamic = cpp_array(10010);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  while ((scanf("%lf%lf%lf%lf", (&ax), (&ay), (&bx), (&by)) != EOF))
  {
    scanf("%d", (&n));
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%lf%lf%lf%lf", (&x[i]), (&y[i]), (&a[i]), (&w[i]));
        i += 1;
      }
    }
    var v: dynamic = cpp_uninitialized();
    v.clear();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        ax -= x[i];
        ay -= y[i];
        bx -= x[i];
        by -= y[i];
        var vv: dynamic = get(a[i], w[i]);
        v.push_back(vv);
        ax += x[i];
        ay += y[i];
        bx += x[i];
        by += y[i];
        i += 1;
      }
    }
    sort(v.begin(), v.end());
    reverse(v.begin(), v.end());
    scanf("%d", (&k));
    if ((k >= v.size()))
    {
      printf("0.00000\n");
    } else
    {
      printf("%.5lf\n", v[k]);
    }
  }
  return 0;
}
