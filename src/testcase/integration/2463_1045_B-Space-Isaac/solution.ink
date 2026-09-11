// Translated from solution.cpp.

func input() -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  read(res);
  {
  }
  return res;
}

func input_seq(b: dynamic, e: dynamic) -> dynamic
{
  generate(b, e, input);
}

func main() -> dynamic
{
  iostream.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = input();
  var m: dynamic = (2 * input());
  for (var elem: dynamic in a)
  {
    elem = (2 * input());
  }
  var parts: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = a[i];
      var y: dynamic = ( ((i == 0)) ? (a.back() - m) : a[(i - 1)]);
      parts.push_back((((x - y)) / 2));
      parts.push_back((((x - y)) / 2));
      i += 1;
    }
  }
  var orig_sz: dynamic = int64_t((parts).size());
  {
    var i: dynamic = 0;
    while ((i < (2 * orig_sz)))
    {
      parts.push_back(parts[i]);
      i += 1;
    }
  }
  var mana: dynamic = cpp_construct((3 * orig_sz));
  var L: dynamic = -1;
  var R: dynamic = -1;
  {
    var i: dynamic = 1;
    while ((i < (3 * orig_sz)))
    {
      if ((i <= R))
      {
        mana[i] = min(((R - i) + 1), mana[(((R + L) - i) + 1)]);
      }
      while (cpp_binary(cpp_binary(((i + mana[i]) < int64_t((parts).size())), "and", (((i - 1) - mana[i]) >= 0)), "and", (parts[(i + mana[i])] == parts[((i - 1) - mana[i])])))
      {
        mana[i] += 1;
      }
      if ((((i + mana[i]) - 1) > R))
      {
        R = ((i + mana[i]) - 1);
        L = (i - mana[i]);
      }
      i += 1;
    }
  }
  var bad: dynamic = cpp_uninitialized();
  var curval: dynamic = a.back();
  {
    var i: dynamic = orig_sz;
    while ((i < (2 * orig_sz)))
    {
      if (((2 * mana[i]) >= orig_sz))
      {
        bad.push_back((((2 * curval)) % m));
      }
      curval += parts[cpp_update(i, "++")];
    }
  }
  sort(bad.begin(), bad.end());
  bad.resize((unique(bad.begin(), bad.end()) - bad.begin()));
  write(int64_t((bad).size()), "\n");
  for (var x: dynamic in bad)
  {
    write((x / 2), " ");
  }
  write("\n");
  return 0;
}
