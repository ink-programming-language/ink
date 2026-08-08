// Translated from solution.cpp.

var waiting = cpp_array(3);

var q: dynamic;

var c = cpp_array(100010);

var maxx = -1;

func main()
{
  var i: dynamic;
  var n: dynamic;
  var k = [0];
  var cnt = [0];
  var t = [0];
  scanf("%d", (&k[0]));
  scanf("%d", (&k[1]));
  scanf("%d", (&k[2]));
  scanf("%lld", (&t[0]));
  scanf("%lld", (&t[1]));
  scanf("%lld", (&t[2]));
  scanf("%d", (&n));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%lld", (&c[i]));
      q.push(make_pair((-c[i]), make_pair(i, 3)));
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    var p = q.top();
    q.pop();
    var i = p.second.second;
    if ((i == 3))
    {
      if ((cnt[0] < k[0]))
      {
        cnt[0] += 1;
        q.push(make_pair((p.first - t[0]), make_pair(p.second.first, 0)));
      } else
      {
        waiting[0].push(p.second.first);
      }
    } else
    {
      cnt[i] -= 1;
      if ((!waiting[i].empty()))
      {
        cnt[i] += 1;
        var person = waiting[i].front();
        waiting[i].pop();
        q.push(make_pair((p.first - t[i]), make_pair(person, i)));
      }
      if ((i < 2))
      {
        if ((cnt[(i + 1)] < k[(i + 1)]))
        {
          cnt[(i + 1)] += 1;
          q.push(make_pair((p.first - t[(i + 1)]), make_pair(p.second.first, (i + 1))));
        } else
        {
          waiting[(i + 1)].push(p.second.first);
        }
      }
      if ((i == 2))
      {
        maxx = max(maxx, ((-p.first) - c[p.second.first]));
      }
    }
  }
  printf("%lld\n", maxx);
}
