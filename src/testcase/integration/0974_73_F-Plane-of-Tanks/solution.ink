// Translated from solution.cpp.

var pi = acos(-1.0);

var ax: dynamic;

var ay: dynamic;

var bx: dynamic;

var by: dynamic;

func dis(x: dynamic, y: dynamic)
{
  return sqrt(((x * x) + (y * y)));
}

func get(a: dynamic, w: dynamic)
{
  var v = 0;
  var num = 100;
  {
    var i = 0;
    while ((i <= 100))
    {
      var x = (ax + ((((bx - ax)) * i) / cpp_cast(num)));
      var y = (ay + ((((by - ay)) * i) / cpp_cast(num)));
      var p = (atan2(y, x) - a);
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
      var t = (if (((w < 1e-5))) (if ((i == 0)) 0 else 1e10) else (p / w));
      v = max(v, (hypot((x - ax), (y - ay)) / t));
      i += 1;
    }
  }
  return v;
}

var x = cpp_array(10010);

var y = cpp_array(10010);

var a = cpp_array(10010);

var w = cpp_array(10010);

func main()
{
  var n: dynamic;
  var k: dynamic;
  while ((scanf("%lf%lf%lf%lf", (&ax), (&ay), (&bx), (&by)) != EOF))
  {
    scanf("%d", (&n));
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%lf%lf%lf%lf", (&x[i]), (&y[i]), (&a[i]), (&w[i]));
        i += 1;
      }
    }
    var v: dynamic;
    v.clear();
    {
      var i = 0;
      while ((i < n))
      {
        ax -= x[i];
        ay -= y[i];
        bx -= x[i];
        by -= y[i];
        var vv = get(a[i], w[i]);
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
