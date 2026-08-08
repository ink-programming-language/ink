// Translated from solution.cpp.

func input()
{
  var res: dynamic;
  read(res);
  {
  }
  return res;
}

func input_seq(b: dynamic, e: dynamic)
{
  generate(b, e, input);
}

func main()
{
  iostream.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n = input();
  var m = (2 * input());
  for (var elem in a)
  {
    elem = (2 * input());
  }
  var parts: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var x = a[i];
      var y = (if ((i == 0)) (a.back() - m) else a[(i - 1)]);
      parts.push_back((((x - y)) / 2));
      parts.push_back((((x - y)) / 2));
      i += 1;
    }
  }
  var orig_sz = int64_t((parts).size());
  {
    var i = 0;
    while ((i < (2 * orig_sz)))
    {
      parts.push_back(parts[i]);
      i += 1;
    }
  }
  var mana = cpp_construct((3 * orig_sz));
  var L = -1;
  var R = -1;
  {
    var i = 1;
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
  var bad: dynamic;
  var curval = a.back();
  {
    var i = orig_sz;
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
  for (var x in bad)
  {
    write((x / 2), " ");
  }
  write("\n");
  return 0;
}
