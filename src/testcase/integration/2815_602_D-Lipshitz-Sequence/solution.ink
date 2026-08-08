// Translated from solution.cpp.

func main()
{
  var done: dynamic;
  var grads1: dynamic;
  var grads2: dynamic;
  var sols: dynamic;
  var n: dynamic;
  var q: dynamic;
  scanf("%d%d", (&n), (&q));
  var vals: dynamic;
  vals.resize(n);
  {
    var c = 0;
    while ((c < n))
    {
      scanf("%d", (&vals[c]));
      c += 1;
    }
  }
  {
    var c = 0;
    while ((c < (n - 1)))
    {
      grads1.push_back(pair(abs((vals[(c + 1)] - vals[c])), c));
      c += 1;
    }
  }
  grads2 = grads1;
  sort(grads2.begin(), grads2.end());
  sols.resize((n - 1));
  var left = cpp_construct((n - 1));
  var right = cpp_construct((n - 1));
  {
    var c = (n - 2);
    while ((c >= 0))
    {
      var l = done.upper_bound(grads2[c].second);
      var u = done.lower_bound((grads2[c].second + 1));
      if ((l == done.begin()))
      {
        left[grads2[c].second] = 0;
      } else
      {
        left[grads2[c].second] = (*(cpp_update(l, "--")));
      }
      if ((u == done.end()))
      {
        right[grads2[c].second] = (n - 1);
      } else
      {
        right[grads2[c].second] = (*u);
      }
      done.insert(grads2[c].second);
      done.insert((grads2[c].second + 1));
      c -= 1;
    }
  }
  var i: dynamic;
  var j: dynamic;
  while (cpp_update(q, "--"))
  {
    scanf("%d%d", (&i), (&j));
    var r = 0;
    i -= 1;
    j -= 1;
    {
      var c = i;
      while ((c < j))
      {
        r += ((cpp_cast(grads1[c].first) * ((min(right[c], j) - c))) * (((c + 1) - max(left[c], i))));
        c += 1;
      }
    }
    write(r, "\n");
  }
  return 0;
}
