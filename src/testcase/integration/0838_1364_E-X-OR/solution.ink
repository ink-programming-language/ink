// Translated from solution.cpp.

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var mp: dynamic;

func ask(l: dynamic, r: dynamic)
{
  if ((mp.find([l, r]) != mp.end()))
  {
    return mp[[l, r]];
  }
  write("? ", l, " ", r, "\n");
  var x: dynamic;
  read(x);
  mp[[l, r]] = x;
  mp[[r, l]] = x;
  return x;
}

func main()
{
  ios_base.sync_with_stdio(0);
  istream.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  read(n);
  var grr: dynamic;
  grr.clear();
  {
    var i = 1;
    while ((i <= n))
    {
      grr.push_back(i);
      i += 1;
    }
  }
  shuffle(grr.begin(), grr.end(), rng);
  var a = grr[0];
  var b = grr[1];
  var val = ask(a, b);
  {
    var i = 2;
    while ((i < n))
    {
      var c = grr[i];
      var x = ask(b, c);
      if ((x == val))
      {
        val = ask(a, c);
        b = c;
        i += 1;
        continue;
      }
      if ((val > x))
      {
        a = c;
        val = x;
      }
      i += 1;
    }
  }
  var nuller: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      if (((i == a) || (i == b)))
      {
        i += 1;
        continue;
      }
      var l = ask(a, i);
      var r = ask(b, i);
      if ((l == r))
      {
        i += 1;
        continue;
      }
      if ((l < r))
      {
        nuller = a;
        break;
      } else
      {
        nuller = b;
        break;
      }
      i += 1;
    }
  }
  var gr: dynamic;
  gr.clear();
  {
    var i = 1;
    while ((i <= n))
    {
      if ((i == nuller))
      {
        gr.push_back(0);
        i += 1;
        continue;
      }
      gr.push_back(ask(nuller, i));
      i += 1;
    }
  }
  write("! ");
  for (var it in gr)
  {
    write(it, " ");
  }
  write("\n");
}
