// Translated from solution.cpp.

var USE_MATH_DEFINES: dynamic = cpp_expression("#def");

var INF: dynamic = cpp_expression("#define _U");

var tx: dynamic = [+0, +1, +0, -1];

var ty: dynamic = [-1, +0, +1, +0];

var EPS: dynamic = 1e-12;

class MagicalCircle
{
  var pos: dynamic = cpp_uninitialized();
  var dist: dynamic = cpp_uninitialized();
  var time: dynamic = cpp_uninitialized();
  func MagicalCircle(p: dynamic, d: dynamic, t: dynamic) -> dynamic
  {
      self->pos = cpp_construct(p);
      self->dist = cpp_construct(d);
      self->time = cpp_construct(t);
    }
  func operator_less(m: dynamic) -> dynamic
  {
      return (pos < m.pos);
    }
  func operator_greater(m: dynamic) -> dynamic
  {
      return (pos > m.pos);
    }
  func operator_less(num: dynamic) -> dynamic
  {
      return (pos < num);
    }
  func operator_greater(num: dynamic) -> dynamic
  {
      return (pos > num);
    }
}

class State
{
  var pos: dynamic = cpp_uninitialized();
  var time: dynamic = cpp_uninitialized();
  func State(p: dynamic, t: dynamic) -> dynamic
  {
      self->pos = cpp_construct(p);
      self->time = cpp_construct(t);
    }
  func operator_less(s: dynamic) -> dynamic
  {
      return (time < s.time);
    }
  func operator_greater(s: dynamic) -> dynamic
  {
      return (time > s.time);
    }
}

func main() -> dynamic
{
  var distance: dynamic = cpp_uninitialized();
  var total_magical_circles: dynamic = cpp_uninitialized();
  while ((~scanf("%d %d", (&distance), (&total_magical_circles))))
  {
    var magical_circles: dynamic = cpp_uninitialized();
    {
      var circle_idx: dynamic = 0;
      while ((circle_idx < total_magical_circles))
      {
        var pos: dynamic = cpp_uninitialized();
        var dist: dynamic = cpp_uninitialized();
        var time: dynamic = cpp_uninitialized();
        scanf("%d %d %d", (&pos), (&dist), (&time));
        magical_circles.push_back(MagicalCircle(pos, dist, time));
        circle_idx += 1;
      }
    }
    magical_circles.push_back(MagicalCircle(distance, 0, 0));
    sort(magical_circles.begin(), magical_circles.end());
    var que: dynamic = cpp_uninitialized();
    que.push(State(0, 0));
    var dp: dynamic = cpp_uninitialized();
    while ((!que.empty()))
    {
      var s: dynamic = que.top();
      que.pop();
      if ((dp.find(s.pos) != dp.end()))
      {
        continue;
      }
      dp[s.pos] = s.time;
      var idx: dynamic = (lower_bound(magical_circles.begin(), magical_circles.end(), s.pos) - magical_circles.begin());
      if ((magical_circles[idx].pos == s.pos))
      {
        var next_time: dynamic = (s.time + magical_circles[idx].time);
        var next_pos: dynamic = (s.pos + magical_circles[idx].dist);
        que.push(State(next_pos, next_time));
        var next_time2: dynamic = (s.time + 1);
        var next_pos2: dynamic = (s.pos + 1);
        que.push(State(next_pos2, next_time2));
      } else
      {
        var next_time: dynamic = (s.time + ((magical_circles[idx].pos - s.pos)));
        var next_pos: dynamic = magical_circles[idx].pos;
        que.push(State(next_pos, next_time));
      }
    }
    printf("%d\n", dp[distance]);
  }
}
