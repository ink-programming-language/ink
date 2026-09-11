// Translated from solution.cpp.

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var mp: dynamic = cpp_uninitialized();

func ask(l: dynamic, r: dynamic) -> dynamic
{
  if ((mp.find([l, r]) != mp.end()))
  {
    return mp[[l, r]];
  }
  write("? ", l, " ", r, "\n");
  var x: dynamic = cpp_uninitialized();
  read(x);
  mp[[l, r]] = x;
  mp[[r, l]] = x;
  return x;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  istream.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var grr: dynamic = cpp_uninitialized();
  grr.clear();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      grr.push_back(i);
      i += 1;
    }
  }
  shuffle(grr.begin(), grr.end(), rng);
  var a: dynamic = grr[0];
  var b: dynamic = grr[1];
  var val: dynamic = ask(a, b);
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      var c: dynamic = grr[i];
      var x: dynamic = ask(b, c);
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
  var nuller: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (((i == a) || (i == b)))
      {
        i += 1;
        continue;
      }
      var l: dynamic = ask(a, i);
      var r: dynamic = ask(b, i);
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
  var gr: dynamic = cpp_uninitialized();
  gr.clear();
  {
    var i: dynamic = 1;
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
  for (var it: dynamic in gr)
  {
    write(it, " ");
  }
  write("\n");
}
