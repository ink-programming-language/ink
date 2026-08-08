// Translated from solution.cpp.

var USE_MATH_DEFINES = cpp_expression("#def");

var INF = cpp_expression("#define _U");

var tx = [+0, +1, +0, -1];

var ty = [-1, +0, +1, +0];

var EPS = 1e-12;

class MagicalCircle
{
  var pos: dynamic;
  var dist: dynamic;
  var time: dynamic;
  func MagicalCircle(p: dynamic, d: dynamic, t: dynamic)
  {
      this->pos = cpp_construct(p);
      this->dist = cpp_construct(d);
      this->time = cpp_construct(t);
    }
  func operator_less(m: dynamic)
  {
      return (pos < m.pos);
    }
  func operator_greater(m: dynamic)
  {
      return (pos > m.pos);
    }
  func operator_less(num: dynamic)
  {
      return (pos < num);
    }
  func operator_greater(num: dynamic)
  {
      return (pos > num);
    }
}

class State
{
  var pos: dynamic;
  var time: dynamic;
  func State(p: dynamic, t: dynamic)
  {
      this->pos = cpp_construct(p);
      this->time = cpp_construct(t);
    }
  func operator_less(s: dynamic)
  {
      return (time < s.time);
    }
  func operator_greater(s: dynamic)
  {
      return (time > s.time);
    }
}

func main()
{
  var distance: dynamic;
  var total_magical_circles: dynamic;
  while ((~scanf("%d %d", (&distance), (&total_magical_circles))))
  {
    var magical_circles: dynamic;
    {
      var circle_idx = 0;
      while ((circle_idx < total_magical_circles))
      {
        var pos: dynamic;
        var dist: dynamic;
        var time: dynamic;
        scanf("%d %d %d", (&pos), (&dist), (&time));
        magical_circles.push_back(MagicalCircle(pos, dist, time));
        circle_idx += 1;
      }
    }
    magical_circles.push_back(MagicalCircle(distance, 0, 0));
    sort(magical_circles.begin(), magical_circles.end());
    var que: dynamic;
    que.push(State(0, 0));
    var dp: dynamic;
    while ((!que.empty()))
    {
      var s = que.top();
      que.pop();
      if ((dp.find(s.pos) != dp.end()))
      {
        continue;
      }
      dp[s.pos] = s.time;
      var idx = (lower_bound(magical_circles.begin(), magical_circles.end(), s.pos) - magical_circles.begin());
      if ((magical_circles[idx].pos == s.pos))
      {
        var next_time = (s.time + magical_circles[idx].time);
        var next_pos = (s.pos + magical_circles[idx].dist);
        que.push(State(next_pos, next_time));
        var next_time2 = (s.time + 1);
        var next_pos2 = (s.pos + 1);
        que.push(State(next_pos2, next_time2));
      } else
      {
        var next_time = (s.time + ((magical_circles[idx].pos - s.pos)));
        var next_pos = magical_circles[idx].pos;
        que.push(State(next_pos, next_time));
      }
    }
    printf("%d\n", dp[distance]);
  }
}
