// Translated from solution.cpp.

func in_cpp()
{
  var a: dynamic;
  scanf("%d", (&a));
  return a;
}

func gcm(a: dynamic, b: dynamic)
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

func calc_rev(x: dynamic)
{
  var ret = 0;
  while (x)
  {
    var a = (x % 10);
    ret = ((ret * 10) + a);
    x /= 10;
  }
  return ret;
}

var rev = cpp_array(100010);

var lucky: dynamic;

func a_reva(a: dynamic)
{
  var g = gcm(a, rev[a]);
  var b = (a / g);
  var c = (rev[a] / g);
  return make_pair(b, c);
}

func invert_pint(t: dynamic)
{
  return make_pair(t.second, t.first);
}

func main()
{
  {
    var i = (1);
    while ((i <= (100000)))
    {
      rev[i] = calc_rev(i);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i <= (100000)))
    {
      lucky[a_reva(i)].push_back(i);
      i += 1;
    }
  }
  var ans = make_pair(-1, -1);
  var ans_fact = ((cpp_cast((1001001001)) * (1001001001)));
  var maxx = in_cpp();
  var maxy = in_cpp();
  var w = in_cpp();
  var bar = maxy;
  var ltickets = 0;
  {
    var x = (1);
    while ((x <= (maxx)))
    {
      var hoge = lucky[invert_pint(a_reva(x))];
      var ind = distance(hoge.begin(), upper_bound(hoge.begin(), hoge.end(), bar));
      ltickets += ind;
      while ((ltickets >= w))
      {
        if (((cpp_cast(x) * bar) < ans_fact))
        {
          ans = make_pair(x, bar);
          ans_fact = (cpp_cast(x) * bar);
        }
        var fuga = lucky[invert_pint(a_reva(bar))];
        var ind2 = distance(fuga.begin(), upper_bound(fuga.begin(), fuga.end(), x));
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
