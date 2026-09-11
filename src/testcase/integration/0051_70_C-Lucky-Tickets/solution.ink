// Translated from solution.cpp.

func in_cpp() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  scanf("%d", (&a));
  return a;
}

func gcm(a: dynamic, b: dynamic) -> dynamic
{
  if ((b > a))
  {
    return gcm(b, a);
  }
  if (((a % b) == 0))
  {
    return b;
  } else
  {
    return gcm(b, (a % b));
  }
}

func calc_rev(x: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  while (x)
  {
    var a: dynamic = (x % 10);
    ret = ((ret * 10) + a);
    x /= 10;
  }
  return ret;
}

var rev: dynamic = cpp_array(100010);

var lucky: dynamic = cpp_uninitialized();

func a_reva(a: dynamic) -> dynamic
{
  var g: dynamic = gcm(a, rev[a]);
  var b: dynamic = (a / g);
  var c: dynamic = (rev[a] / g);
  return make_pair(b, c);
}

func invert_pint(t: dynamic) -> dynamic
{
  return make_pair(t.second, t.first);
}

func main() -> dynamic
{
  {
    var i: dynamic = (1);
    while ((i <= (100000)))
    {
      rev[i] = calc_rev(i);
      i += 1;
    }
  }
  {
    var i: dynamic = (1);
    while ((i <= (100000)))
    {
      lucky[a_reva(i)].push_back(i);
      i += 1;
    }
  }
  var ans: dynamic = make_pair(-1, -1);
  var ans_fact: dynamic = ((cpp_cast((1001001001)) * (1001001001)));
  var maxx: dynamic = in_cpp();
  var maxy: dynamic = in_cpp();
  var w: dynamic = in_cpp();
  var bar: dynamic = maxy;
  var ltickets: dynamic = 0;
  {
    var x: dynamic = (1);
    while ((x <= (maxx)))
    {
      var hoge: dynamic = lucky[invert_pint(a_reva(x))];
      var ind: dynamic = distance(hoge.begin(), upper_bound(hoge.begin(), hoge.end(), bar));
      ltickets += ind;
      while ((ltickets >= w))
      {
        if (((cpp_cast(x) * bar) < ans_fact))
        {
          ans = make_pair(x, bar);
          ans_fact = (cpp_cast(x) * bar);
        }
        var fuga: dynamic = lucky[invert_pint(a_reva(bar))];
        var ind2: dynamic = distance(fuga.begin(), upper_bound(fuga.begin(), fuga.end(), x));
        ltickets -= ind2;
        bar -= 1;
      }
      x += 1;
    }
  }
  if ((ans.first == -1))
  {
    puts("-1");
  } else
  {
    printf("%d %d\n", ans.first, ans.second);
  }
  return 0;
}
